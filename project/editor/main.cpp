#define NOMINMAX
#include <Windows.h>

#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "framework/input/InputFrameWork.h"

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

	std::string mainWindowName = "ShootingGameEditor";
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

	// InputInterfaceの初期化
	std::unique_ptr<QFE::INPUT::InputInterface> inputInterface =
		QFE::FRAMEWORK::CreateInputInterface(mainWindow, hInstance);

	// シーンマネージャの初期化
	QFE::SCENE::SceneManager sceneManager;
	sceneManager.Initialize();
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

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
	QFE::GRAPHIC::PSOHandle psoHandle;
	QFE::FRAMEWORK::CreateGraphicPSO(graphicEngine.get(), shaderPairHandle,
		QFE::GRAPHIC::RasterizerType::Default,
		QFE::GRAPHIC::BlendMode::kBlendModeNormal,
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
				graphicEngine.get(),
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
				graphicEngine.get(), modelDataMap[modelName].meshes[0].vertices.GetInternalVector(),
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
			graphicEngine.get(), offScreenRenderTargetHandle, 1280, 720, DXGI_FORMAT_R16G16B16A16_FLOAT);
		renderTargets.push_back(offScreenRenderTargetHandle);
	}

	// ビューポートとシザー矩形の作成
	QFE::GRAPHIC::ViewPortHandle viewportHandle;
	QFE::FRAMEWORK::CreateViewport(graphicEngine.get(), viewportHandle, 1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle;
	QFE::FRAMEWORK::CreateScissorRect(graphicEngine.get(), scissorRectHandle, 0, 0, 1280, 720);

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle;
	QFE::FRAMEWORK::CreateUAVBuffer(graphicEngine.get(), uavBufferHandle, 1280, 720, L"UAVBuffer");

	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::GRAPHIC::RTPSOHandle rtpsoHandle;
	QFE::FRAMEWORK::CreateRayTracingPSO(graphicEngine.get(), rtpsoHandle, rtDirName, "ShadowRaytracing.hlsl");

	// エディタ用のシーンテクスチャの作成
	QFE::GRAPHIC::RenderTargetHandle sceneRenderTargetHandle;
	QFE::FRAMEWORK::CreateOffScreenRenderTarget(graphicEngine.get(), sceneRenderTargetHandle, 1280, 720, DXGI_FORMAT_R8G8B8A8_UNORM);
	QFE::GRAPHIC::DirectXResourceHandle sceneTextureHandle;
	QFE::FRAMEWORK::GetRenderResourceHandle(graphicEngine.get(), sceneRenderTargetHandle, sceneTextureHandle);

	// シーン描画用のImGuiテクスチャIDを取得
	ImTextureID sceneTextureId =
		QFE::FRAMEWORK::GetImGuiTextureIdFromResourceHandle(graphicEngine.get(), sceneTextureHandle);

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
			inputInterface->Update();

			gameEditor.Update();
			// デバッグカメラ操作処理
			if (gameEditor.GetActiveCameraType() == QFE::EDITOR::EditorCameraType::DebugCamera) {
				// デバッグカメラの操作
				float moveSpeed = 0.5f;
				float rotateSpeed = 0.01f;
				// キーボード入力による移動
				if (inputInterface->GetKeyPress("Up")) {
					cameraTransform.translate.z += moveSpeed;
				}
				if (inputInterface->GetKeyPress("Down")) {
					cameraTransform.translate.z -= moveSpeed;
				}
				if (inputInterface->GetKeyPress("Left")) {
					cameraTransform.translate.x -= moveSpeed;
				}
				if (inputInterface->GetKeyPress("Right")) {
					cameraTransform.translate.x += moveSpeed;
				}
			}

			// カメラのビュー行列と投影行列を取得
			QFE::MATH::Matrix4x4 viewProj = QFE::MATH::Matrix4x4::MakeIndentity4x4();
			if (gameEditor.GetActiveCameraType() == QFE::EDITOR::EditorCameraType::DebugCamera) {


				QFE::MATH::Matrix4x4 viewMatrix = QFE::MATH::Matrix4x4::MakeAffineMatrix(cameraTransform).Inverse();
				QFE::MATH::Matrix4x4 projectionMatrix = QFE::MATH::Matrix4x4::MakePerspectiveFovMatrix(
					3.14159f / 4.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
				viewProj = QFE::MATH::Matrix4x4::Multiply(viewMatrix, projectionMatrix);
			} else {
				entityManager.Each<QFE::SCENE::CameraComponent>([&](uint32_t entityId, QFE::SCENE::CameraComponent& cameraComp) {
					if (cameraComp.isMainCamera) {
						if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
							QFE::MATH::EulerTransform& cameraTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
							cameraComp.viewMatrix = QFE::MATH::Matrix4x4::MakeAffineMatrix(cameraTransform).Inverse();
							if (cameraComp.top_ - cameraComp.bottom_ != 0.0f) {
								cameraComp.aspectRatio_ = fabsf((cameraComp.right_ - cameraComp.left_) / (cameraComp.top_ - cameraComp.bottom_));
							} else {
								cameraComp.aspectRatio_ = 1.0f; // デフォルトのアスペクト比
							}
							cameraComp.projectionMatrix = QFE::MATH::Matrix4x4::MakePerspectiveFovMatrix(
								cameraComp.fovY_, cameraComp.aspectRatio_, cameraComp.nearZ_, cameraComp.farZ_);

							viewProj = QFE::MATH::Matrix4x4::Multiply(cameraComp.viewMatrix, cameraComp.projectionMatrix);
						}
					}
					});
			}

			// 各エンティティのModelRenderComponentを更新
			std::vector<std::pair<QFE::GRAPHIC::BLASHandle, QFE::MATH::Matrix4x4>> raytracingInstances;
			entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
				modelRenderComp.canRender = false;
				// TransformComponentを取得して、EulerTransformを更新する
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId) == false) {
					modelRenderComp.renderErrorMessage = "Missing TransformComponent for entity: " + std::to_string(entityId);
					return;
				}
				QFE::MATH::EulerTransform objTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;

				if (entityManager.HasComponent < QFE::SCENE::AnimationComponent>(entityId)) {
					QFE::SCENE::AnimationComponent& animationComp = entityManager.GetComponent<QFE::SCENE::AnimationComponent>(entityId);
					objTransform.translate += animationComp.transform.translate;
					objTransform.rotate += animationComp.transform.rotate;
					objTransform.scale += animationComp.transform.scale;
				}

				QFE::GRAPHIC::DirectXResourceAllocator* resourceAllocator = graphicEngine->GetDirectXResourceAllocator();
				QFE::GRAPHIC::DirectXResourceHandle transformMatrixBufferHandle =
					resourceAllocator->AllocateConstantBuffer<TransformationMatrix>();
				QFE::FRAMEWORK::UpdateObject3dWVPMatrix(graphicEngine.get(), transformMatrixBufferHandle, objTransform, viewProj);
				modelRenderComp.transformMatrixBufferHandle =
					static_cast<uint32_t>(transformMatrixBufferHandle);

				// マテリアルの更新
				if (entityManager.HasComponent<QFE::SCENE::MaterialComponent>(entityId) == false) {
					modelRenderComp.renderErrorMessage = "Missing MaterialComponent for entity: " + std::to_string(entityId);
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
					if (loadModelVertexBufferFunc(modelRenderComp.modelName)) {
						modelRenderComp.vertexResourceHandle = static_cast<uint32_t>(vertexBufferMap[modelRenderComp.modelName]);
					} else {
						modelRenderComp.renderErrorMessage = "Failed to load vertex buffer for model: " + modelRenderComp.modelName;
						return;
					}
				}

				// テクスチャの更新
				QFE::GRAPHIC::DirectXResourceHandle textureHandle;
				QFE::FRAMEWORK::GetWhite1x1TextureHandle(graphicEngine.get(), textureHandle);
				modelRenderComp.textureResourceHandle = static_cast<uint32_t>(textureHandle);

				// レイトレーシングインスタンスの作成
				if (blasHandleMap.find(modelRenderComp.modelName) == blasHandleMap.end()) {
					if (!loadBlasFunc(modelRenderComp.modelName)) {
						modelRenderComp.renderErrorMessage = "Failed to load BLAS for model: " + modelRenderComp.modelName;
						return;
					}
				}
				raytracingInstances.push_back({
					blasHandleMap[modelRenderComp.modelName],
					QFE::MATH::Matrix4x4::MakeAffineMatrix(entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform)
					});

				// レンダリング可能
				modelRenderComp.canRender = true;
				modelRenderComp.renderErrorMessage = "";
				});

			QFE::FRAMEWORK::UpdateBLASInstanceBuffer(graphicEngine.get(), raytracingInstances);

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
				QFE::FRAMEWORK::DrawGraphicPSO(graphicEngine.get(), psoHandle, viewportHandle, scissorRectHandle,
					static_cast<QFE::GRAPHIC::DirectXResourceHandle>(modelRenderComp.vertexResourceHandle),
					modelRootResources, renderTargets);
				});

			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources(3);
			for (int i = 0; i < 3; ++i) {
				QFE::FRAMEWORK::GetRenderResourceHandle(graphicEngine.get(), renderTargets[i], rayTracingRootResources[i]);
			}
			QFE::FRAMEWORK::DrawRayTracingPSO(graphicEngine.get(), rtpsoHandle, uavBufferHandle, rayTracingRootResources);

			QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle;
			if (QFE::FRAMEWORK::GetDepthStencilResourceHandle(graphicEngine.get(), depthStencilHandle)) {
				QFE::FRAMEWORK::SetRenderTarget(graphicEngine.get(), depthStencilHandle, { QFE::GRAPHIC::RenderTargetHandle::SwapChain });
			}

			guiManager->PostDraw();
			graphicEngine->PostDraw();
			sceneManager.EndFrame();

			fpsCounter->FrameEnd();
			inputInterface->EndFrame();
		}
	}



	sceneManager.Shutdown();
	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();

	return 0;
}