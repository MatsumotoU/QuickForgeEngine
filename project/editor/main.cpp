#define NOMINMAX
#include <Windows.h>

#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"

#include "window/GameWindowManager.h"
#include "graphics/D3D12GraphicEngine.h"

#include "gui/D3D12GuiManager.h"
#include "camera/CameraManager.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "core/loger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

#include "GameEditor.h"

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	std::string mainWindowName = "Runtime";
	uint32_t mainWindowHeight = 720;
	uint32_t mainWindowWidth = 1280;
	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	std::unique_ptr<QFE::GameWindowManager> gameWindowManager =
		QFE::FRAMEWORK::CreateWindowManager(mainWindowName, mainWindowWidth, mainWindowHeight);
	HWND mainWindow = QFE::FRAMEWORK::GetWindowHandle(gameWindowManager.get(), mainWindowName);

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine =
		QFE::FRAMEWORK::CreateGraphicEngine(mainWindow);
	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager =
		QFE::FRAMEWORK::CreateGuiManager(graphicEngine.get(), mainWindow);

	// FPSカウンターの初期化
	std::unique_ptr<QFE::FPSCounter> fpsCounter = std::make_unique<QFE::FPSCounter>();
	fpsCounter->Reset();

	// カメラマネージャの初期化とカメラの作成
	QFE::CAMERA::CameraManager cameraManager;
	cameraManager.Initialize();
	QFE::CAMERA::CameraHandle cameraHandle = cameraManager.CreateCamera(0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 100.0f, 0.45f);

	// シーンマネージャの初期化
	QFE::SCENE::SceneManager sceneManager;
	sceneManager.Initialize();
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	// JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
	std::wstring selectedFilePath;
	if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
		mainWindow,
		L"JSON Files", L"*.json",
		selectedFilePath)) {
		// Entityの生成
		sceneManager.LoadCurrentSceneFromJson(QFE::ConvertString(selectedFilePath));
	}


	std::string psDirName = "engine/resources/shaders/ps/";
	std::string vsDirName = "engine/resources/shaders/vs/";
	std::string rtDirName = "engine/resources/shaders/rt/";

	//====================
	// ここから描画の準備
	//====================

	// シェーダーペアを生成
	QFE::GRAPHIC::ShaderPairHandle shaderPairHandle;
	QFE::FRAMEWORK::CreateShaderPair(graphicEngine.get(), vsDirName, psDirName, "Object3d.GBuffer.VS.hlsl", "Object3d.GBuffer.PS.hlsl", shaderPairHandle);
	// パイプラインステートオブジェクトを生成
	QFE::GRAPHIC::PSOHandle psoHandle = graphicEngine->CreatePipelineStateObject(
		shaderPairHandle, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::DepthStencilDescType::Default);

	QFE::MATH::Transform cameraTransform;
	cameraTransform.translate = { 0.0f, 20.0f, -20.0f };
	cameraTransform.rotate = { 0.8f, 0.0f, 0.0f };

	// Vertexバッファの作成とモデルデータの読み込み
	QFE::ASSET::AssimpModelLoader modelLoader;
	modelLoader.Initialize();
	std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle> vertexBufferMap;
	std::unordered_map<std::string, QFE::ASSET::ModelData> modelDataMap;
	std::vector<std::string> modelNames;
	entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
		modelNames.push_back(modelRenderComp.modelName);
		});
	std::string modelDir = "resources/";
	for (std::string modelName : modelNames) {
		modelDataMap[modelName] = modelLoader.LoadModel(modelDir + modelName + ".obj");
		vertexBufferMap[modelName] = graphicEngine->CreateVertexBuffer(modelDataMap[modelName].meshes[0].vertices.GetInternalVector(), "VertexBuffer");
	}

	// オフスクリーンレンダーターゲットの作成
	std::vector<QFE::GRAPHIC::RenderTargetHandle> renderTargets;
	for (int i = 0; i < 3; ++i) {
		QFE::GRAPHIC::RenderTargetHandle offScreenRenderTargetHandle =
			graphicEngine->CreateOffScreenRenderTarget(1280, 720, DXGI_FORMAT_R16G16B16A16_FLOAT);
		renderTargets.push_back(offScreenRenderTargetHandle);
	}

	// ビューポートとシザー矩形の作成
	QFE::GRAPHIC::ViewPortHandle viewportHandle = graphicEngine->CreateViewPort(1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle = graphicEngine->CreateScissorRect(0, 0, 1280, 720);

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle = graphicEngine->CreateUAVBuffer(1280, 720, L"UAVBuffer");

	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::GRAPHIC::RTPSOHandle rtpsoHandle = graphicEngine->CreateRayTracingPipelineStateObject(
		rtDirName, "ShadowRaytracing.hlsl");

	// BLASの作成とBLASインスタンスの作成
	std::unordered_map<std::string, QFE::GRAPHIC::BLASHandle> blasHandleMap;
	for (const auto& [modelName, modelData] : modelDataMap) {
		std::vector<QFE::MATH::Vector3> objectVertices;
		objectVertices = QFE::FRAMEWORK::GetModelVertexPositions(
			modelData.meshes[0].vertices.GetInternalVector().data(), modelData.meshes[0].vertices.GetInternalVector().size());
		QFE::GRAPHIC::BLASHandle blasHandle = graphicEngine->CreateBLAS(
			objectVertices, modelName);
		blasHandleMap[modelName] = blasHandle;
	}

	// エディタ用のシーンテクスチャの作成
	QFE::GRAPHIC::RenderTargetHandle sceneRenderTargetHandle =
		graphicEngine->CreateOffScreenRenderTarget(1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM);
	QFE::GRAPHIC::DirectXResourceHandle sceneTextureHandle =
		graphicEngine->GetRenderTargetTexture(sceneRenderTargetHandle);
	D3D12_GPU_DESCRIPTOR_HANDLE sceneTextureGPUHandle = graphicEngine->GetSRVDescriptorGPUHandle(sceneTextureHandle);

	ImTextureID sceneTextureId = static_cast<ImTextureID>(static_cast<uintptr_t>(sceneTextureGPUHandle.ptr));

	QFE::EDITOR::GameEditor gameEditor;
	gameEditor.Initialize(&sceneManager, sceneTextureId, mainWindow);

	// メインループ
	while (gameWindowManager->IsWindowActive()) {
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			// WM_QUITメッセージが来たらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			fpsCounter->FrameStart();

			gameEditor.Update();

			QFE::MATH::Matrix4x4 viewProj =
				cameraManager.GetViewProjectionMatrix(cameraHandle, cameraTransform, QFE::CAMERA::CameraType::Perspective);

			// 各エンティティのModelRenderComponentを更新
			std::vector<QFE::GRAPHIC::RaytracingInstance> raytracingInstances;
			entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
				modelRenderComp.canRender = false;
				// TransformComponentを取得して、Transformを更新する
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId) == false) {
					return;
				}
				QFE::MATH::Transform& objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
				QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetResourceAllocator();
				QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
					resourceAllocator->AllocateConstantBuffer<TransformationMatrix>();
				QFE::FRAMEWORK::UpdateObject3dWVPMatrix(graphicEngine.get(), transformMatrixBufferHandle, objTransform, viewProj);
				modelRenderComp.transformMatrixBufferHandle =
					static_cast<uint32_t>(transformMatrixBufferHandle);

				// マテリアルの更新
				if (entityManager.HasComponent<QFE::SCENE::MaterialComponent>(entityId) == false) {
					return;
				}
				QFE::GRAPHIC::DirectXResourceHandle materialBufferHandle =
					resourceAllocator->AllocateConstantBuffer<Material>();
				Material* materialData = graphicEngine->GetConstantBufferData<Material>(materialBufferHandle);
				QFE::SCENE::MaterialComponent& materialComp = entityManager.GetComponent<QFE::SCENE::MaterialComponent>(entityId);
				materialData->color = materialComp.albedoColor;
				modelRenderComp.materialResourceHandle = static_cast<uint32_t>(materialBufferHandle);

				// 頂点バッファの更新
				if (vertexBufferMap.find(modelRenderComp.modelName) != vertexBufferMap.end()) {
					modelRenderComp.vertexResourceHandle = static_cast<uint32_t>(vertexBufferMap[modelRenderComp.modelName]);
				} else {
					return;
				}

				// テクスチャの更新
				QFE::GRAPHIC::DirectXResourceHandle textureHandle =
					graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyWhite1x1Texture);
				modelRenderComp.textureResourceHandle = static_cast<uint32_t>(textureHandle);

				// レイトレーシングインスタンスの作成
				raytracingInstances.push_back({
					blasHandleMap[modelRenderComp.modelName],
					QFE::MATH::Matrix4x4::MakeAffineMatrix(entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform)
					});

				// レンダリング可能
				modelRenderComp.canRender = true;
				});

			graphicEngine->UpdateBLASInstanceTransform(raytracingInstances);

			graphicEngine->PreDraw();
			guiManager->PreDraw();

			gameEditor.Draw();

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
				graphicEngine->TestOffScreenDraw(
					psoHandle, viewportHandle, scissorRectHandle,
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.vertexResourceHandle),
					modelRootResources, renderTargets);
				});

			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources = {
				graphicEngine->GetRenderTargetTexture(renderTargets[0]),
				graphicEngine->GetRenderTargetTexture(renderTargets[1]),
				graphicEngine->GetRenderTargetTexture(renderTargets[2])
			};

			graphicEngine->RayTracingDispatch(rtpsoHandle, uavBufferHandle, rayTracingRootResources,sceneTextureHandle);

			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager->PostDraw();
			graphicEngine->PostDraw();
			sceneManager.EndFrame();

			fpsCounter->FrameEnd();
		}
	}



	sceneManager.Shutdown();
	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();

	return 0;
}