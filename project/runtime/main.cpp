#define NOMINMAX
#include <Windows.h>

#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/script/WindowsScriptWorkFrame.h"
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
#include "script/ScriptInstance.h"
#include "script/EngineContextForScript.h"
#include "input/InputInterface.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	std::string mainWindowName = "ShootingGameRuntime";
	uint32_t mainWindowHeight = 720;
	uint32_t mainWindowWidth = 1280;
	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	std::unique_ptr<QFE::GameWindowManager> gameWindowManager =
		QFE::FRAMEWORK::CreateWindowManager(mainWindowName, mainWindowWidth, mainWindowHeight);
	HWND mainWindow = QFE::FRAMEWORK::GetWindowHandle(gameWindowManager.get(), mainWindowName);

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine =
		QFE::FRAMEWORK::CreateGraphicEngine(mainWindow);

	// FPSカウンターの初期化
	std::unique_ptr<QFE::FPSCounter> fpsCounter = std::make_unique<QFE::FPSCounter>();
	fpsCounter->Reset();

	// シーンマネージャの初期化
	QFE::SCENE::SceneManager sceneManager;
	sceneManager.Initialize();
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	// InputInterfaceの初期化
	std::unique_ptr<QFE::INPUT::InputInterface> inputInterface = 
		QFE::FRAMEWORK::CreateInputInterface(mainWindow, hInstance);

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
		sceneManager.LoadCurrentSceneFromJson(QFE::ConvertString(selectedFilePath));
	}

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
	std::unordered_map<std::string, QFE::GRAPHIC::BLASHandle> blasHandleMap;
	std::unordered_map<std::string, QFE::ASSET::ModelData> modelDataMap;
	std::string modelDir = "resources/";
	// モデルの読み込みとBLASを作る関数
	std::function<bool(const std::string&)> loadModelVertexBufferFunc =
		[&](const std::string& modelName) {
		QFE::ASSET::ModelData modelData;
		if (modelLoader.LoadModel(modelDir + modelName + ".obj", modelData)) {
			modelDataMap[modelName] = modelData;
			vertexBufferMap[modelName] =
				graphicEngine->CreateVertexBuffer(modelDataMap[modelName].meshes[0].vertices.GetInternalVector(), "VertexBuffer");
			return true;
		}
		return false;
		};
	std::function<bool(const std::string&)> loadBlasFunc =
		[&](const std::string& modelName) {
		QFE::ASSET::ModelData modelData;
		if (modelLoader.LoadModel(modelDir + modelName + ".obj", modelData)) {
			modelDataMap[modelName] = modelData;
			std::vector<QFE::MATH::Vector3> objectVertices;
			objectVertices = QFE::FRAMEWORK::GetModelVertexPositions(
				modelDataMap[modelName].meshes[0].vertices.GetInternalVector().data(),
				modelDataMap[modelName].meshes[0].vertices.GetInternalVector().size());
			QFE::GRAPHIC::BLASHandle blasHandle = graphicEngine->CreateBLAS(objectVertices, modelName);
			blasHandleMap[modelName] = blasHandle;
			return true;
		}
		return false;
		};

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

	// TestDll.dllをロードしてスクリプト関数の目録を取得
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance;
	std::wstring filePath;
	scriptInstance = QFE::FRAMEWORK::LoadWindowsScriptInstance(L"GameLogics.dll", "GetManifest");

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

			float deltaTime = fpsCounter->GetDeltaTime();

			// シューティングプレイヤーの実行
			entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent& shootingPlayerComp) {
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::Transform& playerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					float speed = shootingPlayerComp.speed;
					float targetRotateZ = 0.0f;
					 
					// プレイヤーの移動処理
					if (inputInterface->GetKeyPress("Up")) {
						shootingPlayerComp.velocity.z = speed * deltaTime;
					}
					if (inputInterface->GetKeyPress("Down")) {
						shootingPlayerComp.velocity.z = -speed * deltaTime;
					}
					if (inputInterface->GetKeyPress("Left")) {
						if (targetRotateZ < -0.1f) {
							playerTransform.rotate.z = 10.0f;
						}
						targetRotateZ = 0.7f;
						shootingPlayerComp.velocity.x = -speed * deltaTime;
					}
					if (inputInterface->GetKeyPress("Right")) {
						if (targetRotateZ > 0.1f) {
							playerTransform.rotate.z = -10.0f;
						}
						targetRotateZ = -0.7f;
						shootingPlayerComp.velocity.x = speed * deltaTime;
					}

					// プレイヤーの位置を更新
					playerTransform.translate += shootingPlayerComp.velocity;
					shootingPlayerComp.velocity.x *= shootingPlayerComp.damping; // 減衰を適用
					shootingPlayerComp.velocity.z *= shootingPlayerComp.damping; // 減衰を適用

					// プレイヤーの回転処理（Z軸回転）
					playerTransform.rotate.z = QFE::MATH::SimpleEaseIn(playerTransform.rotate.z, targetRotateZ, 0.1f);

					// プレイヤーの射撃処理
					if (inputInterface->GetKeyPress("Shot")) {

					}
					// プレイヤーのボム処理
					if (inputInterface->GetKeyRelease("Shot")) {

					}
				}
				});

			// スクリプト関数の実行
			entityManager.Each<QFE::SCENE::ScriptComponent>([&](uint32_t entityId, QFE::SCENE::ScriptComponent& scriptComp) {
				// 名前から関数を探す
				for (auto& scripts : scriptInstance->scripts) {
					if (scripts.functionName == scriptComp.scriptFunctionName) {
						scriptComp.scriptFunctionIndex = &scripts - &scriptInstance->scripts[0];
						break;
					}
				}
				// もし関数が見つからなかった場合は、スクリプト関数の実行をスキップする
				if (scriptComp.scriptFunctionIndex >= scriptInstance->scripts.size()) {
					return;
				}
				// 関数の実行
				scriptInstance->scripts[scriptComp.scriptFunctionIndex].functionPtr(entityId, 0.016f, &entityManager);
				});

			// カメラのビュー行列と投影行列を取得
			QFE::MATH::Matrix4x4 viewProj = QFE::MATH::Matrix4x4::MakeIndentity4x4();
			entityManager.Each<QFE::SCENE::CameraComponent>([&](uint32_t entityId, QFE::SCENE::CameraComponent& cameraComp) {
				if (cameraComp.isMainCamera) {
					if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
						QFE::MATH::Transform& cameraTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
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

			// 各エンティティのModelRenderComponentを更新
			std::vector<QFE::GRAPHIC::RaytracingInstance> raytracingInstances;
			entityManager.Each<QFE::SCENE::ModelRenderComponent>([&](uint32_t entityId, QFE::SCENE::ModelRenderComponent& modelRenderComp) {
				modelRenderComp.canRender = false;
				// TransformComponentを取得して、Transformを更新する
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId) == false) {
					modelRenderComp.renderErrorMessage = "Missing TransformComponent for entity: " + std::to_string(entityId);
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
				QFE::GRAPHIC::DirectXResourceHandle textureHandle =
					graphicEngine->GetBuiltInTextureHandle(QFE::GRAPHIC::BuiltInTextureType::DummyWhite1x1Texture);
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

			graphicEngine->UpdateBLASInstanceTransform(raytracingInstances);

			graphicEngine->PreDraw();

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

			graphicEngine->TestRayTracing(rtpsoHandle, uavBufferHandle, rayTracingRootResources);

			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			graphicEngine->PostDraw();
			sceneManager.EndFrame();

			fpsCounter->FrameEnd();
			inputInterface->EndFrame();
		}
	}

	sceneManager.Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
	return 0;
}