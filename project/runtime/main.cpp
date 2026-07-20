#define NOMINMAX
#include <Windows.h>

#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/script/WindowsScriptWorkFrame.h"
#include "framework/input/InputFrameWork.h"
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

	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager =
		QFE::FRAMEWORK::CreateGuiManager(graphicEngine.get(), mainWindow);

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
	QFE::GRAPHIC::PSOHandle psoHandle;
	QFE::FRAMEWORK::CreateGraphicPSO(graphicEngine.get(), shaderPairHandle,
		QFE::GRAPHIC::RasterizerType::Default, QFE::GRAPHIC::BlendMode::kBlendModeNormal,
		QFE::GRAPHIC::DepthStencilDescType::Default, psoHandle);
	std::vector<D3D12_ROOT_PARAMETER_TYPE> rootParameterTypes;
	QFE::FRAMEWORK::GetGraphicPSORootParameterTypeList(graphicEngine.get(), psoHandle, rootParameterTypes);

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
	for (int i = 0; i < 4; ++i) {
		QFE::GRAPHIC::RenderTargetHandle offScreenRenderTargetHandle;
		QFE::FRAMEWORK::CreateOffScreenRenderTarget(
			graphicEngine.get(), offScreenRenderTargetHandle, 1280, 720, DXGI_FORMAT_R16G16B16A16_FLOAT);
		renderTargets.push_back(offScreenRenderTargetHandle);
	}

	// ビューポートとシザー矩形の作成
	QFE::GRAPHIC::ViewPortHandle viewportHandle;
	QFE::FRAMEWORK::CreateViewport(graphicEngine.get(), viewportHandle,1280, 720);
	QFE::GRAPHIC::ScissorRectHandle scissorRectHandle;
	QFE::FRAMEWORK::CreateScissorRect(graphicEngine.get(), scissorRectHandle, 0, 0, 1280, 720);

	// UAVバッファの作成とルートリソースの設定
	QFE::GRAPHIC::DirectXResourceHandle uavBufferHandle;
	QFE::FRAMEWORK::CreateUAVBuffer(graphicEngine.get(), uavBufferHandle, 1280, 720, L"UAVBuffer");

	// レイトレーシングパイプラインステートオブジェクトの作成
	QFE::GRAPHIC::RTPSOHandle rtpsoHandle;
	QFE::FRAMEWORK::CreateRayTracingPSO(graphicEngine.get(), rtpsoHandle, rtDirName, "ShadowRaytracing.hlsl");

	// TestDll.dllをロードしてスクリプト関数の目録を取得
	std::unique_ptr<QFE::SCRIPT::WindowsScriptInstance> scriptInstance;
	std::wstring filePath;
	scriptInstance = QFE::FRAMEWORK::LoadWindowsScriptInstance(L"GameLogics.dll", "GetManifest");

	// assetDir
	std::string assetDir = "resources/";

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

			// AutoScroll
			entityManager.Each<QFE::STG::AutoScrollComponent>([&](uint32_t entityId, QFE::STG::AutoScrollComponent& autoScrollComp) {
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::EulerTransform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					if (autoScrollComp.distance > 0.0f) {
						autoScrollComp.distance -= autoScrollComp.speed * deltaTime;
						transform.translate.z += autoScrollComp.speed * deltaTime;
					}
				}
				});

			// シューティングプレイヤーの実行
			entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent& shootingPlayerComp) {
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::EulerTransform& playerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					float speed = shootingPlayerComp.speed;
					float targetRotateZ = 0.0f;
					float rotatePower = 1.0f;

					if (inputInterface->GetKeyPress("Slow")) {
						speed *= 0.5f;
						rotatePower *= 0.5f;
					}

					// プレイヤーの移動処理
					if (inputInterface->GetKeyPress("Up")) {
						shootingPlayerComp.velocity.z = speed * deltaTime;
					}
					if (inputInterface->GetKeyPress("Down")) {
						shootingPlayerComp.velocity.z = -speed * deltaTime;
					}
					if (inputInterface->GetKeyPress("Left")) {

						targetRotateZ = 0.7f;
						shootingPlayerComp.velocity.x = -speed * deltaTime;
					}
					if (inputInterface->GetKeyPress("Right")) {
						targetRotateZ = -0.7f;
						shootingPlayerComp.velocity.x = speed * deltaTime;
					}

					if (inputInterface->GetKeyTrigger("Left")) {
						if (playerTransform.rotate.z < -0.6f) {
							playerTransform.rotate.z = -5.0f;
						}
					}
					if (inputInterface->GetKeyTrigger("Right")) {
						if (playerTransform.rotate.z > 0.6f) {
							playerTransform.rotate.z = 5.0f;
						}
					}

					// プレイヤーの位置を更新
					playerTransform.translate += shootingPlayerComp.velocity;
					shootingPlayerComp.velocity.x *= shootingPlayerComp.damping; // 減衰を適用
					shootingPlayerComp.velocity.z *= shootingPlayerComp.damping; // 減衰を適用

					// プレイヤーの回転処理（Z軸回転）
					playerTransform.rotate.z = QFE::MATH::SimpleEaseIn(playerTransform.rotate.z, targetRotateZ * rotatePower, 0.1f);

					// プレイヤーの射撃処理
					if (shootingPlayerComp.shootTimer > 0.0f) {
						shootingPlayerComp.shootTimer -= deltaTime;
					} else {
						shootingPlayerComp.shootTimer = 0.0f;
					}
					if (shootingPlayerComp.bombTimer > 0.0f) {
						shootingPlayerComp.bombTimer -= deltaTime;
					} else {
						shootingPlayerComp.bombTimer = 0.0f;
					}

					if (inputInterface->GetKeyPress("Shot")) {
						if (shootingPlayerComp.shootTimer > 0.0f) {
							return;
						}

						uint32_t bulletEntityId =
							sceneManager.LoadEntityOnCurrentSceneFromJsonObject(assetDir + shootingPlayerComp.bulletPrefabName);

						if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId)) {
							QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
							bulletTransform.translate = playerTransform.translate + shootingPlayerComp.bulletSpawnOffset;
						}

						shootingPlayerComp.shootTimer = shootingPlayerComp.shootInterval;
					}
					// プレイヤーのボム処理
					if (inputInterface->GetKeyRelease("Shot")) {
						uint32_t bombEntityId =
							sceneManager.LoadEntityOnCurrentSceneFromJsonObject(assetDir + shootingPlayerComp.bombPrefabName);

						if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bombEntityId)) {
							QFE::MATH::EulerTransform& bombTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bombEntityId).transform;
							bombTransform.translate = playerTransform.translate + shootingPlayerComp.bombSpawnOffset;
						}
					}
				}
				});

			// 敵の処理
			std::vector<QFE::MATH::Vector3> playerPositionsE;
			entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent& shootingPlayerComp) {
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::EulerTransform& playerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					playerPositionsE.push_back(playerTransform.translate);
				}
				});
			entityManager.Each<QFE::STG::EnemyAIComponent>([&](uint32_t entityId, QFE::STG::EnemyAIComponent& enemyAIComp) {
				enemyAIComp.shotTimer -= deltaTime;
				if (enemyAIComp.shotTimer <= 0.0f) {
					enemyAIComp.shotTimer = enemyAIComp.shotInterval;
					uint32_t bulletEntityId =
						sceneManager.LoadEntityOnCurrentSceneFromJsonObject(assetDir + enemyAIComp.bulletName);
					if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId) &&
						entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
						QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
						QFE::MATH::EulerTransform& enemyTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
						bulletTransform.translate = enemyTransform.translate;
					}
					// プレイヤーの位置に向かって弾丸を発射する
					if (!playerPositionsE.empty()) {
						QFE::MATH::Vector3 targetPosition = playerPositionsE[0]; // 最初のプレイヤーの位置をターゲットにする
						if (entityManager.HasComponent<QFE::STG::BulletComponent>(bulletEntityId) &&
							entityManager.HasComponent<QFE::SCENE::TransformComponent>(bulletEntityId)) {
							QFE::STG::BulletComponent& bulletComp = entityManager.GetComponent<QFE::STG::BulletComponent>(bulletEntityId);
							QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(bulletEntityId).transform;
							bulletComp.dir = (targetPosition - bulletTransform.translate).Normalize();
						}
					}
				}
				});

			// 弾丸の処理
			entityManager.Each<QFE::STG::BulletComponent>([&](uint32_t entityId, QFE::STG::BulletComponent& bulletComp) {
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::EulerTransform& bulletTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					bulletTransform.translate += bulletComp.dir * bulletComp.speed * deltaTime;
					bulletComp.lifeTimeMax -= deltaTime;
					if (bulletComp.lifeTimeMax <= 0.0f) {
						entityManager.RemoveEntity(entityId);
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

			// コライダーの処理
			std::vector<uint32_t>colliderEntityIds;
			std::map<uint32_t, QFE::SCENE::SphereColliderComponent> colliderComponents;
			std::map<uint32_t, QFE::SCENE::TransformComponent> transformComponents;
			entityManager.Each<QFE::SCENE::SphereColliderComponent>([&](uint32_t entityId, QFE::SCENE::SphereColliderComponent& colliderComp) {
				if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					return;
				}
				colliderEntityIds.push_back(entityId);
				colliderComponents[entityId] = colliderComp;
				transformComponents[entityId] = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
				});
			// 球のコライダー同士の衝突判定	
			for (size_t i = 0; i < colliderEntityIds.size(); ++i) {
				uint32_t entityIdA = colliderEntityIds[i];
				QFE::SCENE::SphereColliderComponent& colliderA = colliderComponents[entityIdA];
				QFE::MATH::EulerTransform& transformA = transformComponents[entityIdA].transform;
				for (size_t j = i + 1; j < colliderEntityIds.size(); ++j) {
					uint32_t entityIdB = colliderEntityIds[j];
					QFE::SCENE::SphereColliderComponent& colliderB = colliderComponents[entityIdB];
					QFE::MATH::EulerTransform& transformB = transformComponents[entityIdB].transform;

					// タグマスクの判定
					if ((colliderA.mask & colliderB.mask) != 0) {
						continue; // 衝突判定をスキップ
					}

					// 衝突判定
					float distance = (transformA.translate - transformB.translate).Length();
					if (distance < (colliderA.radius + colliderB.radius)) {
						// 衝突が発生した場合の処理
						if (entityManager.HasComponent<QFE::STG::HealthComponent>(entityIdA)) {
							QFE::STG::HealthComponent& healthCompA = entityManager.GetComponent<QFE::STG::HealthComponent>(entityIdA);
							healthCompA.health -= 1;
							if (healthCompA.health <= 0) {
								entityManager.RemoveEntity(entityIdA);
							}
						} else {
							entityManager.RemoveEntity(entityIdA);
						}
						if (entityManager.HasComponent<QFE::STG::HealthComponent>(entityIdB)) {
							QFE::STG::HealthComponent& healthCompB = entityManager.GetComponent<QFE::STG::HealthComponent>(entityIdB);
							healthCompB.health -= 1;
							if (healthCompB.health <= 0) {
								entityManager.RemoveEntity(entityIdB);
							}
						} else {
							entityManager.RemoveEntity(entityIdB);
						}
					}
				}
			}

			// 移動制限コンポーネントの処理
			entityManager.Each<QFE::STG::MoveLimitComponent>([&](uint32_t entityId, QFE::STG::MoveLimitComponent& moveLimitComp) {
				if (moveLimitComp.autoScrollDistance > 0.0f) {
					float scrollAmount = moveLimitComp.autoScrollSpeed.Length() * deltaTime;
					moveLimitComp.center += moveLimitComp.autoScrollSpeed * deltaTime;
					moveLimitComp.autoScrollDistance -= scrollAmount;
				}

				if (!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					return;
				}
				QFE::MATH::EulerTransform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
				QFE::MATH::Vector3 minLimit = moveLimitComp.center + moveLimitComp.minLimit;
				QFE::MATH::Vector3 maxLimit = moveLimitComp.center + moveLimitComp.maxLimit;
				transform.translate.x = std::clamp(transform.translate.x, minLimit.x, maxLimit.x);
				transform.translate.y = std::clamp(transform.translate.y, minLimit.y, maxLimit.y);
				transform.translate.z = std::clamp(transform.translate.z, minLimit.z, maxLimit.z);
				});

			// プレイヤー自動トラッキング処理
			std::vector<uint32_t> playerEntityIds;
			std::vector<QFE::MATH::Vector3> playerPositions;
			entityManager.Each<QFE::STG::ShootingPlayerComponent>([&](uint32_t entityId, QFE::STG::ShootingPlayerComponent& shootingPlayerComp) {
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::EulerTransform& playerTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					playerPositions.push_back(playerTransform.translate);
					playerEntityIds.push_back(entityId);
				}
				});
			entityManager.Each<QFE::STG::PlayerTrackingComponent>([&](uint32_t entityId, QFE::STG::PlayerTrackingComponent& autoTrackComp) {
				if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
					QFE::MATH::EulerTransform& transform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
					if (!playerPositions.empty()) {
						// プレイヤーの平均位置を計算
						QFE::MATH::Vector3 averagePosition = { 0.0f, 0.0f, 0.0f };
						for (const auto& pos : playerPositions) {
							averagePosition += pos;
						}
						averagePosition.x /= static_cast<float>(playerPositions.size());
						averagePosition.y /= static_cast<float>(playerPositions.size());
						averagePosition.z /= static_cast<float>(playerPositions.size());
						// オブジェクトの位置をプレイヤーの平均位置に
						if (autoTrackComp.isTrackingX)
						{
							transform.translate.x = averagePosition.x + autoTrackComp.trackingOffsetPos.x;
						}
						if (autoTrackComp.isTrackingY)
						{
							transform.translate.y = averagePosition.y + autoTrackComp.trackingOffsetPos.y;
						}
						if (autoTrackComp.isTrackingZ)
						{
							transform.translate.z = averagePosition.z + autoTrackComp.trackingOffsetPos.z;
						}
					}
					// 回転のトラッキング
					if (autoTrackComp.isTrackingRotation) {
						// プレイヤーの平均位置を計算
						QFE::MATH::Vector3 averagePosition = { 0.0f, 0.0f, 0.0f };
						for (const auto& pos : playerPositions) {
							averagePosition += pos;
						}
						averagePosition.x /= static_cast<float>(playerPositions.size());
						averagePosition.y /= static_cast<float>(playerPositions.size());
						averagePosition.z /= static_cast<float>(playerPositions.size());
						QFE::MATH::Vector3 directionToPlayer = averagePosition - transform.translate;
						directionToPlayer = directionToPlayer.Normalize();
						float targetYaw = atan2f(directionToPlayer.x, directionToPlayer.z);
						float targetPitch = asinf(-directionToPlayer.y);
						targetYaw = std::lerp(targetYaw, autoTrackComp.trackingRotationOffset.y, autoTrackComp.trackingRotationTranspose);
						targetPitch = std::lerp(targetPitch, autoTrackComp.trackingRotationOffset.x, autoTrackComp.trackingRotationTranspose);

						transform.rotate.x = targetPitch;
					}
				}
				});

			// カメラのビュー行列と投影行列を取得
			QFE::MATH::Matrix4x4 viewProj = QFE::MATH::Matrix4x4::MakeIndentity4x4();
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
				materialData->metallic = materialComp.metallic;
				materialData->smoothness = materialComp.smoothness;
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
					modelRootResources,renderTargets, rootParameterTypes);
				});

			std::vector<QFE::GRAPHIC::DirectXResourceHandle> rayTracingRootResources(4);
			for (int i = 0; i < 4; ++i) {
				QFE::FRAMEWORK::GetRenderResourceHandle(graphicEngine.get(), renderTargets[i], rayTracingRootResources[i]);
			}
			QFE::GRAPHIC::DirectXResourceHandle cameraBufferHandle;
			cameraBufferHandle = graphicEngine->GetDirectXResourceAllocator()->AllocateConstantBuffer<CameraForGPU>("CameraBuffer");
			entityManager.Each<QFE::SCENE::CameraComponent>([&](uint32_t entityId, QFE::SCENE::CameraComponent& cameraComp) {
				if (cameraComp.isMainCamera) {
					CameraForGPU* cameraPos = graphicEngine->GetConstantBufferData<CameraForGPU>(cameraBufferHandle);
					if (entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
						QFE::MATH::EulerTransform& cameraTransform = entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId).transform;
						cameraPos->cameraPosition = cameraTransform.translate;
					}
				}
				});

			QFE::FRAMEWORK::DrawRayTracingPSO(
				graphicEngine.get(), rtpsoHandle, uavBufferHandle, cameraBufferHandle, rayTracingRootResources);

			ImGui::Begin("FPS Counter");
			ImGui::Text("FPS: %.2f", fpsCounter->GetAverageFPS());
			ImGui::End();


			QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle;
			if (QFE::FRAMEWORK::GetDepthStencilResourceHandle(graphicEngine.get(), depthStencilHandle)) {
				QFE::FRAMEWORK::SetRenderTarget(graphicEngine.get(), depthStencilHandle,{QFE::GRAPHIC::RenderTargetHandle::SwapChain});
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