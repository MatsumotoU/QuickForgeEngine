#define NOMINMAX
#include <Windows.h>

#include "framework/application/WindowsEngineFramework.h"

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

#include <array>

#include "EngineDefines.h"

#include "../resources/shaders/shaderStructs/hlslTypeToCpp.h"

// GPU と共有するインスタンスメタ構造（HLSL 側の構造と合わせること）

static_assert(sizeof(InstanceMetaCPU) == sizeof(uint32_t) * 4);

// モデル群（modelDataMap）からグローバルバッファを平坦化する。
// - globalUVs: [u0,v0, u1,v1, ...]
// - globalTriIndices: flattened indices [i0,i1,i2, i3,i4,i5, ...] (各 tri は 3 要素)
// - outInstanceMeta: メッシュ（またはメッシュ単位のエントリ）ごとのメタ情報（InstanceID と一致させること）

static bool EnsureBufferCapacityAndUpload(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
	QFE::GRAPHIC::DirectXResourceHandle& inOutHandle,
	const void* data, size_t byteSize, UINT elementStride,
	const std::string& name) {
	if (!graphicEngine) return false;
	auto* device = graphicEngine->GetDirectXDevice();
	auto* rc = graphicEngine->GetDirectXResourceContainer();

	// 新規作成 or 既存サイズ取得
	bool needCreateView = false;
	if (inOutHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) {
		needCreateView = true;
	} else {
		size_t curSize = rc->GetResourceSizeInBytes(inOutHandle);
		if (curSize < byteSize) {
			// 既存バッファでは小さい -> 新規作成して差し替え
			needCreateView = true;
		}
	}

	if (needCreateView) {
		// 新しいバッファを作成してハンドルを差し替える
		QFE::GRAPHIC::DirectXResourceHandle newHandle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (newHandle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(newHandle);
		if (byteSize > 0 && data) {
			void* mapped = rc->GetMappedData<void>(newHandle);
			if (mapped) memcpy(mapped, data, byteSize);
		}
		rc->SetResourceName(newHandle, QFE::ConvertString(name));
		rc->SetResourceStrideInBytes(newHandle, elementStride);

		// SRV を作る（Shader4ComponentMapping を忘れずに）
		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(byteSize / elementStride);
		viewInfo.srvDesc.Buffer.StructureByteStride = elementStride;
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(newHandle, viewInfo);

		// 既存ハンドルがあれば置き換える（リソース解放は container に任せる想定）
		inOutHandle = newHandle;
		return true;
	} else {
		// 既存バッファを再利用してデータ更新だけ行う（Map + memcpy）
		rc->MapResource(inOutHandle);
		if (byteSize > 0 && data) {
			void* mapped = rc->GetMappedData<void>(inOutHandle);
			if (mapped) memcpy(mapped, data, byteSize);
		}
		// strideは念のため再設定
		rc->SetResourceStrideInBytes(inOutHandle, elementStride);
		return true;
	}
}

// GPU に平坦化済データをアップロードして StructuredBuffer (SRV) を作る。
// 成功時に outXXXHandle にリソースハンドルを格納する。
// 注意: CreateResourceView の srvDesc のフィールド名はプロジェクト実装に合わせて調整してください。
static bool UploadGlobalMeshBuffers(
	QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine,
	const std::vector<float>& globalUVs,
	const std::vector<uint32_t>& globalTriIndices,
	const std::vector<InstanceMetaCPU>& instanceMeta,
	QFE::GRAPHIC::DirectXResourceHandle& outUVHandle,
	QFE::GRAPHIC::DirectXResourceHandle& outTriHandle,
	QFE::GRAPHIC::DirectXResourceHandle& outInstanceMetaHandle) {
	if (!graphicEngine) return false;
	auto* device = graphicEngine->GetDirectXDevice();
	auto* rc = graphicEngine->GetDirectXResourceContainer();

	// 1) UV バッファ
	{
		size_t byteSize = globalUVs.size() * sizeof(float);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		float* mapped = rc->GetMappedData<float>(handle);
		if (mapped && !globalUVs.empty()) {
			memcpy(mapped, globalUVs.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("GlobalUVs")));
		// 要素ストライドは float2
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(float) * 2));

		// SRV 作成（Shader4ComponentMapping を明示的に設定）
		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(globalUVs.size() / 2);
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(float) * 2;
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(handle, viewInfo);

		outUVHandle = handle;
	}

	// 2) TriIndices バッファ（flattened uint triplets）
	{
		size_t byteSize = globalTriIndices.size() * sizeof(uint32_t);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		uint32_t* mapped = rc->GetMappedData<uint32_t>(handle);
		if (mapped && !globalTriIndices.empty()) {
			memcpy(mapped, globalTriIndices.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("GlobalTriIndices")));
		// tri ごとの stride は 3 * uint32
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(uint32_t) * 3));

		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(globalTriIndices.size() / 3);
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(uint32_t) * 3;
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(handle, viewInfo);

		outTriHandle = handle;
	}

	// 3) InstanceMeta バッファ
	{
		size_t byteSize = instanceMeta.size() * sizeof(InstanceMetaCPU);
		auto handle = rc->CreateBuffer(device->GetDevice(), byteSize);
		if (handle == QFE::GRAPHIC::DirectXResourceHandle::Invalid) return false;

		rc->MapResource(handle);
		InstanceMetaCPU* mapped = rc->GetMappedData<InstanceMetaCPU>(handle);
		if (mapped && !instanceMeta.empty()) {
			memcpy(mapped, instanceMeta.data(), byteSize);
		}
		rc->SetResourceName(handle, QFE::ConvertString(std::string("InstanceMeta")));
		rc->SetResourceStrideInBytes(handle, static_cast<UINT>(sizeof(InstanceMetaCPU)));

		QFE::GRAPHIC::CreateViewInfo viewInfo{};
		viewInfo.viewType = QFE::GRAPHIC::ViewTypeFlags::ShaderResourceView;
		viewInfo.srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		viewInfo.srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		viewInfo.srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewInfo.srvDesc.Buffer.NumElements = static_cast<UINT>(instanceMeta.size());
		viewInfo.srvDesc.Buffer.StructureByteStride = sizeof(InstanceMetaCPU);
		viewInfo.srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		rc->CreateResourceView(handle, viewInfo);

		outInstanceMetaHandle = handle;
	}

	return true;
}

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	std::string windowName = "ShootingGameRuntime";
	uint32_t windowWidth = 1280;
	uint32_t windowHeight = 720;

	QFE::FRAMEWORK::WindowsQuickForgeEngineSystems engineSystems;
	if (!QFE::FRAMEWORK::CreateWindowsQuickForgeEngineSystems(
		hInstance,
		windowName,
		windowWidth, windowHeight,
		engineSystems)) {
		return -1;
	}
	auto& gameWindowManager = engineSystems.windowManager;
	auto& graphicEngine = engineSystems.graphicEngine;
	auto& guiManager = engineSystems.guiManager;
	auto& inputInterface = engineSystems.inputInterface;
	auto& fpsCounter = engineSystems.fpsCounter;
	QFE::SCENE::SceneManager& sceneManager = *engineSystems.sceneManager;
	QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();

	HWND mainWindow = QFE::FRAMEWORK::GetWindowHandle(engineSystems.windowManager.get(), windowName);

	std::string psDirName = "engine/resources/shaders/ps/";
	std::string vsDirName = "engine/resources/shaders/vs/";
	std::string rtDirName = "engine/resources/shaders/rt/";

	QFE::GRAPHIC::DirectXResourceHandle globalUVHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;
	QFE::GRAPHIC::DirectXResourceHandle globalTriHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;
	QFE::GRAPHIC::DirectXResourceHandle instanceMetaHandle = QFE::GRAPHIC::DirectXResourceHandle::Invalid;

	// JSONファイルの選択ダイアログを表示して、ユーザーにシーンファイルを選択させる
	std::wstring selectedFilePath;
	if (QFE::FRAMEWORK::RequestGetFilePathFromUser(
		mainWindow,
		L"JSON Files", L"*.json",
		selectedFilePath)) {
		// Entityの生成
		engineSystems.sceneManager->LoadCurrentSceneFromJson(QFE::ConvertString(selectedFilePath));
	}

	//====================
	// ここから描画の準備
	//====================

	float allObjectMetallic = 0.0f;
	float allObjectSmoothness = 0.5f;

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
	std::unordered_map<std::string, QFE::GRAPHIC::DirectXResourceHandle> indexBufferMap;
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
	std::function<bool(const std::string&)> loadModelIndexBufferFunc =
		[&](const std::string& modelName) {
		QFE::ASSET::ModelData modelData;
		if (modelLoader.LoadModel(modelDir + modelName + ".obj", modelData)) {
			modelDataMap[modelName] = modelData;
			bool result = QFE::FRAMEWORK::CreateIndexBuffer(
				graphicEngine.get(),
				modelDataMap[modelName].meshes[0].indices.GetInternalVector(),
				modelName, indexBufferMap[modelName]);
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
				modelDataMap[modelName].meshes[0].indices.GetInternalVector(),
				modelName, blasHandleMap[modelName]);
			return result;
		}
		return false;
		};
	
	// テクスチャを読み込む関数
	std::map<std::string, QFE::GRAPHIC::DirectXResourceHandle> textureHandleMap;
	std::map<std::string, uint32_t> textureGpuIndexMap; // テクスチャ名 -> GPU側のインデックスマップ
	uint32_t nextTextureGpuIndex = 2; // 0,1 は BlackCubeMap と White1x1 に予約されているので、次のインデックスは 2 から始める

	QFE::FRAMEWORK::GetBlackCubeMapTextureHandle(graphicEngine.get(), textureHandleMap["BlackCubeMap"]);
	textureGpuIndexMap["BlackCubeMap"] = 0; // GPU側のインデックスを設定
	QFE::FRAMEWORK::GetWhite1x1TextureHandle(graphicEngine.get(), textureHandleMap["White1x1"]);
	textureGpuIndexMap["white1x1"] = 1; // GPU側のインデックスを設定

	std::function<bool(const std::string&)> loadTextureFunc =
		[&](const std::string& textureName) {
		if(textureHandleMap.find(textureName) != textureHandleMap.end()) {
			// すでに読み込まれている場合はスキップ
			return true;
		}

		std::string texturePath = modelDir + textureName;
		QFE::GRAPHIC::DirectXResourceHandle textureHandle;
		bool result = QFE::FRAMEWORK::LoadTextureFromFile(graphicEngine.get(), texturePath, textureHandle);
		if(result) {
			textureHandleMap[textureName] = textureHandle;
			textureGpuIndexMap[textureName] = nextTextureGpuIndex;
			++nextTextureGpuIndex;
		}
		return result;
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
		if (!QFE::FRAMEWORK::ProcessWindowsApplicationMessage()) {
			break;
		}
		if (!QFE::FRAMEWORK::BeginWindowsEngineFrame(engineSystems)) {
			break;
		}

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

			// ゲームに依存しないカメラ更新はSceneFrameworkへ委譲する。
			QFE::MATH::Matrix4x4 viewProj = QFE::FRAMEWORK::UpdateMainCamera(sceneManager);

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
				materialData->metallic = allObjectMetallic;
				materialData->smoothness = allObjectSmoothness;
				materialData->uvTransform = QFE::MATH::Matrix4x4::MakeAffineMatrix(materialComp.uvTransform);
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

				// インデックスバッファの更新
				if (indexBufferMap.find(modelRenderComp.modelName) != indexBufferMap.end()) {
					modelRenderComp.indexResourceHandle = static_cast<uint32_t>(indexBufferMap[modelRenderComp.modelName]);
				} else {
					if (loadModelIndexBufferFunc(modelRenderComp.modelName)) {
						modelRenderComp.indexResourceHandle = static_cast<uint32_t>(indexBufferMap[modelRenderComp.modelName]);
					} else {
						modelRenderComp.renderErrorMessage = "Failed to load index buffer for model: " + modelRenderComp.modelName;
						return;
					}
				}

				// テクスチャの更新
				QFE::GRAPHIC::DirectXResourceHandle textureHandle;
				QFE::FRAMEWORK::GetWhite1x1TextureHandle(graphicEngine.get(), textureHandle);
				if(modelDataMap.find(modelRenderComp.modelName) != modelDataMap.end()) {
					const QFE::ASSET::ModelData& modelData = modelDataMap[modelRenderComp.modelName];
					if (!modelData.meshes.empty() && !modelData.meshes[0].material.textureName.empty()) {
						const std::string& textureFileName = modelData.meshes[0].material.textureName;
						if (loadTextureFunc(textureFileName)) {
							textureHandle = textureHandleMap[textureFileName];
						} else {
							modelRenderComp.renderErrorMessage = "Failed to load texture: " + textureFileName;
							return;
						}
					}
				}
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

			// 1) global arrays とモデル→meta マップを作る
			std::vector<float> globalUVs;
			std::vector<uint32_t> globalTriIndices;
			std::unordered_map<std::string, InstanceMetaCPU> modelMetaMap;

			// modelDataMap に基づいて平坦化（models -> global arrays）
			// BuildGlobalMeshBuffers は modelName -> InstanceMeta を返す
			QFE::FRAMEWORK::BuildGlobalMeshBuffers(
				modelDataMap, textureGpuIndexMap, globalUVs, globalTriIndices, modelMetaMap);

			// 2) raytracingInstances の順に合わせて instanceMeta を並べる
			std::vector<InstanceMetaCPU> instanceMetaAligned;
			instanceMetaAligned.reserve(raytracingInstances.size());

			// 逆引きテーブル：BLASHandle -> modelName
			std::unordered_map<QFE::GRAPHIC::BLASHandle, std::string> blasToModel;
			for (const auto& kv : blasHandleMap) {
				blasToModel[kv.second] = kv.first;
			}

			for (const auto& inst : raytracingInstances) {
				QFE::GRAPHIC::BLASHandle blas = inst.first;
				auto it = blasToModel.find(blas);
				if (it == blasToModel.end()) {
					// 見つからないなら安全なデフォルトを push（デバッグ用ログ推奨）
					InstanceMetaCPU dummy{};
					instanceMetaAligned.push_back(dummy);
					continue;
				}
				const std::string& modelName = it->second;
				auto mit = modelMetaMap.find(modelName);
				if (mit == modelMetaMap.end()) {
					InstanceMetaCPU dummy{};
					instanceMetaAligned.push_back(dummy);
					continue;
				}
				instanceMetaAligned.push_back(mit->second);
			}

			// 3) バッファのサイズ計算と EnsureBufferCapacityAndUpload による使い回しアップロード
			size_t uvBytes = globalUVs.size() * sizeof(float);
			size_t triBytes = globalTriIndices.size() * sizeof(uint32_t);
			size_t metaBytes = instanceMetaAligned.size() * sizeof(InstanceMetaCPU);

			if (!EnsureBufferCapacityAndUpload(graphicEngine.get(), globalUVHandle, globalUVs.data(), uvBytes, sizeof(float) * 2, "GlobalUVs")) {
				assert(false && "Failed to ensure/upload GlobalUVs");
			}
			if (!EnsureBufferCapacityAndUpload(graphicEngine.get(), globalTriHandle, globalTriIndices.data(), triBytes, sizeof(uint32_t) * 3, "GlobalTriIndices")) {
				assert(false && "Failed to ensure/upload GlobalTriIndices");
			}
			if (!EnsureBufferCapacityAndUpload(graphicEngine.get(), instanceMetaHandle, instanceMetaAligned.data(), metaBytes, sizeof(InstanceMetaCPU), "InstanceMeta")) {
				assert(false && "Failed to ensure/upload InstanceMeta");
			}

			// （重要）既存の UploadGlobalMeshBuffers 呼び出しは削除してください（重複）。
			// TLAS 更新、描画へ進む
			QFE::FRAMEWORK::UpdateBLASInstanceBuffer(graphicEngine.get(), raytracingInstances);

			graphicEngine->PreDraw();
			guiManager->PreDraw();

			QFE::FRAMEWORK::DrawSceneModels(
				sceneManager, graphicEngine.get(), psoHandle, viewportHandle,
				scissorRectHandle, renderTargets, rootParameterTypes);

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

			QFE::GRAPHIC::DirectXResourceHandle textureFirstResourceHandle;
			QFE::FRAMEWORK::GetBlackCubeMapTextureHandle(graphicEngine.get(), textureFirstResourceHandle);

			QFE::FRAMEWORK::TestRayTracingPSO(
				graphicEngine.get(), rtpsoHandle, uavBufferHandle,
				cameraBufferHandle,globalTriHandle,globalUVHandle,
				instanceMetaHandle, textureFirstResourceHandle,rayTracingRootResources);


			ImGui::Begin("FPS Counter");
			ImGui::Text("FPS: %.2f", fpsCounter->GetAverageFPS());
			ImGui::DragFloat("All Object Metallic", &allObjectMetallic, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("All Object Smoothness", &allObjectSmoothness, 0.01f, 0.0f, 1.0f);	
			ImGui::End();


			QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle;
			if (QFE::FRAMEWORK::GetDepthStencilResourceHandle(graphicEngine.get(), depthStencilHandle)) {
				QFE::FRAMEWORK::SetRenderTarget(graphicEngine.get(), depthStencilHandle,{QFE::GRAPHIC::RenderTargetHandle::SwapChain});
			}
			QFE::FRAMEWORK::EndWindowsEngineFrame(engineSystems);
	}

	QFE::FRAMEWORK::ShutdownWindowsQuickForgeEngineSystems(engineSystems);
	return 0;
}
