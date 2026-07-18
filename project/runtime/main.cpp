#define NOMINMAX
#include <Windows.h>

#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/script/WindowsScriptWorkFrame.h"
#include "framework/input/InputFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "framework/core/D3D12EngineUtility.h"

#include "window/GameWindowManager.h"
#include "gui/D3D12GuiManager.h"

#include "camera/CameraManager.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "core/loger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"
#include "script/ScriptInstance.h"
#include "script/EngineContextForScript.h"
#include "input/InputInterface.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui.h>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();
	// QuickForgeのセットアップ構造体を作成
	QFE::FRAMEWORK::QuickForgeSetupConfig config;
	config.hInstance = hInstance;
	config.mainWindowHeight = 720;
	config.mainWindowWidth = 1280;
	config.mainWindowName = "ShootingGameRuntime";
	// エンジンシステムの初期化
	std::unique_ptr<QFE::FRAMEWORK::D3D12EngineSystems> engineSystemsPtr =
		std::make_unique<QFE::FRAMEWORK::D3D12EngineSystems>();
	QFE::FRAMEWORK::SetupEngineSystems(engineSystemsPtr.get(), config);

	// mainWindowのハンドルを取得
	HWND mainWindow = QFE::FRAMEWORK::GetWindowHandle(engineSystemsPtr->gameWindowManager_.get(), config.mainWindowName);

	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager =
		QFE::FRAMEWORK::CreateGuiManager(engineSystemsPtr->graphicEngine_.get(), mainWindow);

	// FPSカウンターの初期化
	std::unique_ptr<QFE::FPSCounter> fpsCounter = std::make_unique<QFE::FPSCounter>();
	fpsCounter->Reset();

	std::string psDirName = "engine/resources/shaders/ps/";
	std::string vsDirName = "engine/resources/shaders/vs/";
	std::string rtDirName = "engine/resources/shaders/rt/";

	// JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
	std::wstring selectedFilePath;
	if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
		mainWindow,
		L"JSON Files", L"*.json",
		selectedFilePath)) {
		// Entityの生成
		QFE::FRAMEWORK::LoadSceneFromJson(engineSystemsPtr->sceneManager_.get(), QFE::ConvertString(selectedFilePath));
	}

	//====================
	// ここから描画の準備
	//====================

	// シェーダーペアを生成
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle;
	QFE::FRAMEWORK::CreateShaderPair(engineSystemsPtr->graphicEngine_.get(), vsDirName, psDirName, "Object3d.GBuffer.VS.hlsl", "Object3d.GBuffer.PS.hlsl", shaderPairHandle);
	// パイプラインステートオブジェクトを生成
	QFE::GRAPHIC::PSOHandle psoHandle;
	QFE::FRAMEWORK::CreateGraphicPSO(engineSystemsPtr->graphicEngine_.get(), shaderPairHandle,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::DepthStencilDescType::Default, psoHandle);

	QFE::MATH::EulerTransform cameraTransform;
	cameraTransform.translate = { 0.0f, 20.0f, -20.0f };
	cameraTransform.rotate = { 0.8f, 0.0f, 0.0f };

	// Vertexバッファの作成とモデルデータの読み込み
	QFE::ASSET::AssimpModelLoader modelLoader;
	modelLoader.Initialize();
	std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle> vertexBufferMap;
	std::unordered_map<std::string, QFE::GRAPHIC::BLASHandle> blasHandleMap;
	std::unordered_map<std::string, QFE::ASSET::ModelData> modelDataMap;
	std::string modelDir = "resources/";
	// モデルの読み込みとBLASを作る関数
	std::function<bool(const std::string&)> loadModelVertexBufferFunc =
		[&](const std::string& modelName) {
		QFE::ASSET::ModelData modelData;
		if (modelLoader.LoadModel(modelDir + modelName + ".obj", modelData)) {
			modelDataMap[modelName] = modelData;
			bool result = QFE::FRAMEWORK::CreateVertexBuffer(
				engineSystemsPtr->graphicEngine_.get(),
				modelDataMap[modelName].meshes[0].vertices.GetInternalVector(),
				modelName, vertexBufferMap[modelName]);
			return result;
		}
		return false;
		};
	std::function<bool(const std::string&)> loadBlasFunc =
		[&](const std::string& modelName) {
		QFE::ASSET::ModelData modelData;
		if (modelLoader.LoadModel(modelDir + modelName + ".obj", modelData)) {
			modelDataMap[modelName] = modelData;
			bool result = QFE::FRAMEWORK::CreateBLAS(
				engineSystemsPtr->graphicEngine_.get(), modelDataMap[modelName].meshes[0].vertices.GetInternalVector(),
				modelName, blasHandleMap[modelName]);
			return result;
		}
		return false;
		};

	// オフスクリーンレンダーターゲットの作成
	std::vector<QFE::GRAPHIC::RenderTargetHandle> renderTargets;
	for (int i = 0; i < 3; ++i) {
		QFE::GRAPHIC::RenderTargetHandle offScreenRenderTargetHandle;
		QFE::FRAMEWORK::CreateOffScreenRenderTarget(
			engineSystemsPtr->graphicEngine_.get(), offScreenRenderTargetHandle, 1280, 720, DXGI_FORMAT_R16G16B16A16_FLOAT);
		renderTargets.push_back(offScreenRenderTargetHandle);
	}

	// ビューポートとシザー矩形の作成
	QFE::GRAPHIC::ViewPortHandle viewportHandle;
	QFE::FRAMEWORK::CreateViewport(engineSystemsPtr->graphicEngine_.get(), viewportHandle, 1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle;
	QFE::FRAMEWORK::CreateScissorRect(engineSystemsPtr->graphicEngine_.get(), scissorRectHandle, 0, 0, 1280, 720);

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle;
	QFE::FRAMEWORK::CreateUAVBuffer(engineSystemsPtr->graphicEngine_.get(), uavBufferHandle, 1280, 720, L"UAVBuffer");
	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::GRAPHIC::RTPSOHandle rtpsoHandle;
	QFE::FRAMEWORK::CreateRayTracingPSO(engineSystemsPtr->graphicEngine_.get(), rtpsoHandle, rtDirName, "ShadowRaytracing.hlsl");

	// TestDll.dllをロードしてスクリプト関数の目録を取得
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance;
	std::wstring filePath;
	scriptInstance = QFE::FRAMEWORK::LoadWindowsScriptInstance(L"GameLogics.dll", "GetManifest");

	// assetDir
	std::string assetDir = "resources/";

	// メインループ
	while (engineSystemsPtr->gameWindowManager_.get()->IsWindowActive()) {
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			// WM_QUITメッセージが来たらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			QFE::FRAMEWORK::FrameStart(engineSystemsPtr.get());
			fpsCounter->FrameStart();

			float deltaTime = fpsCounter->GetDeltaTime();

			

			

			QFE::FRAMEWORK::UpdateBLASInstanceBuffer(graphicEngine.get(), raytracingInstances);

			QFE::FRAMEWORK::PreDraw(engineSystemsPtr.get());
			guiManager->PreDraw();

			// モデルのレンダリング
			entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
				if (modelRenderComp.canRender == false) {
					return;
				}

				// ルートリソースの設定
				std::vector<QFE::GRAPHIC::DirectXResourceHandle> modelRootResources = {
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.transformMatrixBufferHandle),
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.materialResourceHandle),
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.textureResourceHandle)
				};
				QFE::FRAMEWORK::DrawGraphicPSO(graphicEngine.get(), psoHandle, viewportHandle, scissorRectHandle,
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.vertexResourceHandle),
					modelRootResources, renderTargets);
				});

			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources(3);
			for (int i = 0; i < 3; ++i) {
				QFE::FRAMEWORK::GetRenderResourceHandle(graphicEngine.get(), renderTargets[i], rayTracingRootResources[i]);
			}
			QFE::FRAMEWORK::DrawRayTracingPSO(graphicEngine.get(), rtpsoHandle, uavBufferHandle, rayTracingRootResources);

			ImGui::Begin("FPS Counter");
			ImGui::Text("FPS: %.2f", fpsCounter->GetAverageFPS());
			ImGui::End();


			QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle;
			if (QFE::FRAMEWORK::GetDepthStencilResourceHandle(graphicEngine.get(), depthStencilHandle)) {
				QFE::FRAMEWORK::SetRenderTarget(graphicEngine.get(), depthStencilHandle, { QFE::GRAPHIC::RenderTargetHandle::SwapChain });
			}
			
			guiManager->PostDraw();
			QFE::FRAMEWORK::PostDraw(engineSystemsPtr.get());

			fpsCounter->FrameEnd();
			QFE::FRAMEWORK::FrameEnd(engineSystemsPtr.get());
		}
	}

	guiManager->Shutdown();
	QFE::FRAMEWORK::ShutdownEngineSystems(engineSystemsPtr.get());
	return 0;
}