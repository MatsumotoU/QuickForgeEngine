#include "engine/include/scene/SceneObject.h"
#include "engine/include/assets/AssetManager.h"
#include <cassert>

#include "engine/include/core/EngineGlobalValue.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"

#include "engine/include/assets/Script/Data/CsharpComponent.h"
#include "engine/include/collider/ColliderManager.h"
#include "engine/include/audio/AudioInterface.h"

#include "engine/include/core/Math/TransformComponent.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/collider/Data/SphereColliderData.h"
#include "engine/include/core/Math/ParentData.h"
#include "engine/include/camera/Data/CameraData.h"
#include "engine/include/camera/Data/BillboardComponent.h"
#include "engine/include/assets/3DModel/Data/SkyboxComponent.h"

#include <fstream>
#include <execution>
#include <nlohmann/json.hpp>

#include "engine/include/assets/3DModel/Loader/AssimpModelLoader.h"
#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/assets/Sprite/Data/SpriteData.h"
#include "engine/include/assets/Particle/Data/ParticleComponent.h"

#include "engine/include/renderer/ModelRenderer.h"
#include "engine/include/renderer/SpriteRenderer.h"
#include "engine/include/renderer/ParticleRenderer.h"
#include "engine/include/renderer/SkyboxRenderer.h"

#include "engine/include/core/EngineDefines.h"

#include "Engine/include/scene/SceneCommand/SceneEntityCommands.h"
#include "engine/include/scene/SceneObject.h"
#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "engine/include/utility/FileSystems/FileUtility.h"

using namespace QFE;

SceneObject::SceneObject() :
	frameStartCommandInvoker_(1.0f),
	updateCommandInvoker_(1.0f),
	preDrawCommandInvoker_(1.0f),
	drawCommandInvoker_(1.0f),
	postDrawCommandInvoker_(1.0f) {
	assetManager_ = nullptr;
	isRequestedExit_ = false;
	sceneName_ = "NewScene";
}

SceneObject::~SceneObject() {
}

void SceneObject::Initialize() {
	QFE_PROFILE_SCOPE;

	assetManager_ = AssetManager::GetInstance();
	assert(assetManager_);
	isRequestedExit_ = false;

	CameraManager::GetInstance()->Initialize();
	isRequestStopScript_ = false;
	isRunningScript_ = false;
	isPauseScript_ = false;

	// スクリプト実行環境を初期化
	csharpScriptExecutor_.Initialize(&entityManager_);

	// コマンドクリア
	frameStartCommandInvoker_.ClearCommands();
	updateCommandInvoker_.ClearCommands();
	preDrawCommandInvoker_.ClearCommands();
	drawCommandInvoker_.ClearCommands();
	postDrawCommandInvoker_.ClearCommands();

	loadEntitiesBinary_.clear();
}

void SceneObject::Update() {
	QFE_PROFILE_SCOPE;

	frameStartCommandInvoker_.ExecuteCommands();

	// ランタイム中のサブモジュールの更新
	if (isRunningScript_ && !isPauseScript_) {
		csharpScriptExecutor_.FrameStart();
		csharpScriptExecutor_.Update();
		csharpScriptExecutor_.FrameEnd();

		PhysicsManager::GetInstance()->Update();
	} else {
		// ランタイム出ない場合はassetManagerのキャッシュをクリアし続ける（ランタイム中はシーンのロードやエンティティの追加・削除が頻繁に行われるため、キャッシュを使用しない）
		assetManager_->ResetCache();
	}

	// 当たり判定更新
	ColliderManager::GetInstance()->Update();

	// スクリプトの当たり判定更新
	if (isRunningScript_ && !isPauseScript_) {
		csharpScriptExecutor_.CollisionUpdate();
	}

	//  ワールド行列更新
	updateCommandInvoker_.AddSystemCommand(std::make_unique<RemakeUniqeIDCommand>(*(GetEntityManager()), uniqueIdManager_));
	updateCommandInvoker_.AddSystemCommand(std::make_unique<WorldTransformationCommand>(*(GetEntityManager())));
	updateCommandInvoker_.AddSystemCommand(std::make_unique<ParentUpdateCommand>(*(GetEntityManager())));
	updateCommandInvoker_.AddSystemCommand(std::make_unique<AllSpriteResizeCommand>(*(GetEntityManager())));

	// 更新後コマンド実行
	updateCommandInvoker_.ExecuteCommands();
}

void SceneObject::PreDraw() {
	QFE_PROFILE_SCOPE;

	// 3Dモデルのカメラ位置更新
	AssetManager* assetManager = AssetManager::GetInstance();
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();

	// 3Dモデルのカメラ位置更新
	entityManager_.Each<ModelHandle>([&](uint32_t entityId, ModelHandle& model) {
		entityId; // 未使用
		const ModelRenderData* modelDataPtr = assetManager->GetModelRenderData(model.handle);
		GpuBufferPool* gpuBufferPool = assetManager->GetGpuBufferPool();
		for (const auto& meshRenderDataHandle : modelDataPtr->meshRenderDataHandles) {
			CameraForGPU* camera = gpuBufferPool->GetConstantBufferData<CameraForGPU>(meshRenderDataHandle.cameraPosBufferHandle);
			camera->cameraPosition = cameraManager->GetMainCamera().GetPosition();
		}
		});

	// システム上絶対やるべき前描画コマンド
	preDrawCommandInvoker_.AddSystemCommand(std::make_unique<BillboardUpdateCommand>(*(GetEntityManager()), cameraManager->GetMainCameraTransform()));
	preDrawCommandInvoker_.AddSystemCommand(std::make_unique<WvpTransformationCommand>(*(GetEntityManager()), *cameraManager));
	preDrawCommandInvoker_.AddSystemCommand(std::make_unique<SpritePivotUpdateCommand>(*(GetEntityManager())));

	// 実行
	preDrawCommandInvoker_.ExecuteCommands();
}

void SceneObject::Draw() {
	QFE_PROFILE_SCOPE;

	// 当たり判定の描画
	ColliderManager::GetInstance()->Draw();

	// スカイボックスの描画
	entityManager_.Each<SkyboxComponent>([](uint32_t entityId, SkyboxComponent& skyboxComp) {
		entityId; // 未使用
		Render::Skybox::DrawSkybox(skyboxComp);
		});

	// 全描画コマンド追加
	drawCommandInvoker_.AddSystemCommand(std::make_unique<AllEntityRenderingCommand>(*(GetEntityManager())));

	// 描画コマンド実行
	drawCommandInvoker_.ExecuteCommands();
}

void SceneObject::PostDraw() {
	QFE_PROFILE_SCOPE;

	// 後描画コマンド実行
	postDrawCommandInvoker_.ExecuteCommands();
}

void SceneObject::EndFrame() {
	QFE_PROFILE_SCOPE;

	if (isRequestStopScript_) {
		if (isRunningScript_) {
			isRunningScript_ = false;
			LoadScene(sceneName_);
		}
		isRequestStopScript_ = false;
	}
	entityManager_.EndFrame();
}

void SceneObject::Finalize() {
	QFE_PROFILE_SCOPE;

	csharpScriptExecutor_.Finalize();
	entityManager_.ResetEntiry();
}

void SceneObject::LoadScene(const std::string& sceneName) {
	QFE_PROFILE_SCOPE;

	std::string sceneNameCopy = sceneName;
	// jsonファイルでない場合は拡張子を付ける
	if (!sceneNameCopy.ends_with(".json") && !sceneNameCopy.ends_with(".scene")) {
		sceneNameCopy += ".json";
	}

	QFE_LOG("LoadScene: " + sceneNameCopy);
	AssetManager* assetManager = AssetManager::GetInstance();
	// GPUバッファの解放
	assetManager->GetGpuBufferPool()->ReleaseAllConstantBuffers();

	entityManager_.ResetEntiry();
	AudioInterface::GetInstance()->StopAllSound();
	csharpScriptExecutor_.ReloadAssembly();
	csharpScriptExecutor_.ResetScripts();

	// Sceneの読み込み
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ifstream ifs(sceneFilePath + sceneNameCopy);
	if (!ifs.is_open()) {
		QFE_LOG("Faild load scene: " + sceneNameCopy, LogLevel::Error);
		CameraManager::GetInstance()->Initialize();
		return;
	}

	// シーンファイルの形式に応じて読み込み
	nlohmann::json sceneJson;
	if (sceneNameCopy.ends_with(".scene")) {
		// バイナリモードで読み込み、JSONに変換
		if (!QFE::FILE::LoadMsgPackToJson(sceneFilePath + sceneNameCopy, sceneJson)) {
			CameraManager::GetInstance()->Initialize();
			return;
		}
	} else {
		// 従来のJSON(テキスト)読み込み
		ifs >> sceneJson;
		ifs.close();
	}

	// Scene名の設定
	if (sceneJson.contains("sceneName")) {
		sceneName_ = sceneJson["sceneName"].get<std::string>();
	} else {
		sceneName_ = "NoNameScene";
	}

	//　Entityの読み込み
	if (!sceneJson.contains("entities")) return;

	for (const auto& entityJson : sceneJson["entities"]) {
		uint32_t entityId = entityManager_.CreateEntity();
		DeserializeEntity(entityId, entityJson);
	}

}

void SceneObject::SaveScene(const std::string& sceneName) {

	QFE_LOG("SaveScene: " + sceneName);
	AssetManager* assetManager = AssetManager::GetInstance();

	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();

	nlohmann::json sceneJson;
	sceneJson["sceneName"] = sceneName;

	for (auto entityId : entities) {
		nlohmann::json entityJson;
		SerializeEntity(entityId, entityJson);
		sceneJson["entities"].push_back(entityJson);
	}

	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ofstream ofs(sceneFilePath + sceneName + ".json");
	ofs << sceneJson.dump(4);
	ofs.close();
}

void QFE::SceneObject::SaveSceneBinary(const std::string& sceneName) {
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("BinarySaveScene: " + sceneName);
#endif // QFE_OPTIMIZE_OFF
	AssetManager* assetManager = AssetManager::GetInstance();

	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();

	nlohmann::json sceneJson;
	sceneJson["sceneName"] = sceneName;

	for (auto entityId : entities) {
		nlohmann::json entityJson;
		SerializeEntity(entityId, entityJson);
		sceneJson["entities"].push_back(entityJson);
	}

	QFE::FILE::SaveJsonAsMsgPack(sceneJson, assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes") + sceneName + ".scene");
}

void SceneObject::ResetScene() {
	uniqueIdManager_.Reset();
	loadEntitiesBinary_.clear();
}

void SceneObject::RunScene() {
	QFE_PROFILE_SCOPE;

	if (!isRunningScript_) {
		loadEntitiesBinary_.clear();

#ifdef QFE_OPTIMIZE_OFF
		MyDebugLog::GetInstance()->DebugLogClear();
		MyDebugLog::GetInstance()->scriptLogs_.clear();
#endif // QFE_OPTIMIZE_OFF

		SaveScene(sceneName_);
		LoadScene(sceneName_);
		isRunningScript_ = true;
		csharpScriptExecutor_.InitializeGameLogic(&entityManager_);
		ColliderManager::GetInstance()->isRunning = true;
	}
}

void SceneObject::PauseScene() {
	isPauseScript_ = true;
}

void SceneObject::ResumeScene() {
	isPauseScript_ = false;
}

void SceneObject::StopScene() {
	QFE_PROFILE_SCOPE;

	if (isRequestStopScript_) { return; }
	isRequestStopScript_ = true;
	ColliderManager::GetInstance()->isRunning = false;
}

void SceneObject::AddEmptyObject() {

	uint32_t entityId = entityManager_.CreateEntity();
	entityManager_.EmplaceComponent<TransformComponent>(entityId, TransformComponent());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = CheckUniqueEntityName("EmptyObject");
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	entityManager_.EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddParticleEmitter(const std::string& modelName, uint32_t maxCount) {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = entityManager_.CreateEntity();
	// Particleコンポーネントを追加
	ParticleComponent particleComponent;
	particleComponent.modelName = modelName;
	particleComponent.maxParticleCount = maxCount;
	particleComponent.vartexBufferHandle = assetManager->LoadModelMesh(modelName);
	particleComponent.textureHandle = assetManager->LoadModelTexture(modelName);
	particleComponent.materialHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<Material>();
	particleComponent.particleGpuBufferHandle = assetManager->GetParticleGpuDataManager()->CreateParticleBuffer(maxCount);
	entityManager_.EmplaceComponent<ParticleComponent>(entityId, particleComponent);

	// TransformコンポーネントとSceneObjectDataコンポーネントを追加
	entityManager_.EmplaceComponent<TransformComponent>(entityId, TransformComponent());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = CheckUniqueEntityName(modelName + "_ParticleEmitter");
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	entityManager_.EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void QFE::SceneObject::AddSkybox(const std::string& skyboxName)
{
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = entityManager_.CreateEntity();
	
	SkyboxComponent skyboxComponent;
	skyboxComponent.textureName = skyboxName;
	skyboxComponent.textureHandle = assetManager->LoadTexture(skyboxName);
	skyboxComponent.materialBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<Material>();
	skyboxComponent.wvpBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<TransformationMatrix>();
	skyboxComponent.vertexBufferHandle = 
		assetManager->GetModelVertexResourceManager()->AssignBox(DirectXCommon::GetInstance()->GetDevice(), true);

	Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(skyboxComponent.materialBufferHandle);
	material->color = { 1.0f,1.0f,1.0f,1.0f };
	material->enableLighting = false;

	TransformationMatrix* transformData = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(skyboxComponent.wvpBufferHandle);
	transformData->World = QFE::Matrix4x4::MakeIndentity4x4();
	transformData->WVP = QFE::Matrix4x4::MakeIndentity4x4();

	entityManager_.EmplaceComponent<SkyboxComponent>(entityId, skyboxComponent);
	
	SceneObjectData sceneObjectData;
	sceneObjectData.name = CheckUniqueEntityName(skyboxName + "_Skybox");
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	entityManager_.EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
	entityManager_.EmplaceComponent<TransformComponent>(entityId, TransformComponent());
}

void SceneObject::AddModel(const std::string& modelName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = entityManager_.CreateEntity();
	ModelHandle modelHandle;
	modelHandle.modelName = modelName;
	modelHandle.handle = assetManager->LoadModel(modelName);
	entityManager_.EmplaceComponent<ModelHandle>(entityId, modelHandle);
	entityManager_.EmplaceComponent<TransformComponent>(entityId, TransformComponent());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = CheckUniqueEntityName(modelName);
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	entityManager_.EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddSprite(const std::string& spriteName, float width, float height, int inEntityId, int layer, Vector2 pivot) {
	AssetManager* assetManager = AssetManager::GetInstance();
	// EntityID決定
	uint32_t entityId;
	if (inEntityId != -1) {
		entityId = static_cast<uint32_t>(inEntityId);
	} else {
		entityId = entityManager_.CreateEntity();
	}
	// SpriteDataの生成
	SpriteData spriteData;

	spriteData.layer = 0;
	spriteData.pivot = pivot;
	if (entityManager_.HasComponentStrage<SpriteData>()) {
		spriteData.layer = static_cast<uint32_t>(entityManager_.GetComponentStrage<SpriteData>().size());
	}
	if (layer != -1) {
		spriteData.layer = static_cast<uint32_t>(layer);
	}
	spriteData.textureName = spriteName;
	spriteData.textureHandle = assetManager->LoadTexture(spriteName);
	Vector2 textureSize = assetManager->GetTextureManager()->GetTextureSize(spriteData.textureHandle);
	spriteData.height = textureSize.y;
	spriteData.width = textureSize.x;
	if (width != 0.0f) {
		textureSize.x = width;
		spriteData.width = width;
	}
	if (height != 0.0f) {
		textureSize.y = height;
		spriteData.height = height;
	}
	spriteData.vertexBufferHandle = assetManager->GetSpriteManager()->CreateVertexBuffer(textureSize.x, textureSize.y);
	spriteData.wvpBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<TransformationMatrix>();
	assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(spriteData.wvpBufferHandle)->WVP = Matrix4x4::MakeIndentity4x4();
	spriteData.materialBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<Material>();
	Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(spriteData.materialBufferHandle);
	material->color = { 1.0f,1.0f,1.0f,1.0f };
	material->enableLighting = false;
	material->uvTransform = Matrix4x4::MakeIndentity4x4();
	spriteData.lightBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<DirectionalLight>();
	DirectionalLight* light = assetManager->GetGpuBufferPool()->GetConstantBufferData<DirectionalLight>(spriteData.lightBufferHandle);
	light->color = { 1.0f,1.0f,1.0f,1.0f };
	light->direction = { 0.0f,-1.0f,0.0f };
	light->intensity = 1.0f;
	// SpriteDataコンポーネントを追加
	entityManager_.EmplaceComponent<SpriteData>(entityId, spriteData);

	// TransformコンポーネントとSceneObjectDataコンポーネントを追加
	entityManager_.EmplaceComponent<TransformComponent>(entityId, TransformComponent());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = CheckUniqueEntityName(spriteName);
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	entityManager_.EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddCsharpScript(uint32_t entityId, const std::string& className) {
	if (!entityManager_.HasComponent<CsharpComponent>(entityId)) {
		CsharpComponent csharpComponent;
		CsharpHandle csharpHandle;
		csharpHandle.className_ = className;
		if (isRunningScript_) {
			csharpScriptExecutor_.CreateScriptInstance(entityId, className);
		}
		csharpComponent.csharpHandles_.push_back(csharpHandle);
		entityManager_.EmplaceComponent<CsharpComponent>(entityId, csharpComponent);

	} else {
		CsharpComponent& csharpComponent = entityManager_.GetComponent<CsharpComponent>(entityId);
		for (const auto& handles : csharpComponent.csharpHandles_) {
			if (handles.className_ == className) {
				QFE_LOG("Csharp class " + className + " is already attached to entity " + std::to_string(entityId), LogLevel::Warning);
				return;
			}
		}

		CsharpHandle csharpHandle;
		csharpHandle.className_ = className;
		csharpComponent.csharpHandles_.push_back(csharpHandle);
		if(isRunningScript_) {
			csharpScriptExecutor_.CreateScriptInstance(entityId, className);
		}
	}
}

uint32_t SceneObject::AddEntity(const std::string& entityName, bool useCache) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	AssetManager* assetManager = AssetManager::GetInstance();
	QFE_LOG("AddEntity: " + entityName);

	// キャッシュからEntityを読み込む
	if (useCache) {
		if (loadEntitiesBinary_.find(entityName) != loadEntitiesBinary_.end()) {
			uint32_t entityId = entityManager_.CreateEntity();
			DeserializeEntity(entityId, nlohmann::json::from_bson(loadEntitiesBinary_[entityName]));
			QFE_LOG("Loaded Entity from cache: " + entityName);
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
			QFE_LOG("AddEntity Time (Cache): " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
			return entityId;
		}
	}

	// Entityの読み込み
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	std::ifstream ifs(sceneFilePath + entityName);
	if (!ifs.is_open()) {
		std::string errorMsg = "FaildOpenFile: " + sceneFilePath + entityName;
		QFE_REPORT_USER_ERROR(errorMsg.c_str(), UserError::DeveloperError);
		return UINT32_MAX;
	}

	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();

	// キャッシュに保存
	if (useCache) {
		loadEntitiesBinary_[entityName] = nlohmann::json::to_bson(sceneJson);
	}
	
	uint32_t entityId = entityManager_.CreateEntity();
	DeserializeEntity(entityId, sceneJson);
	QFE_LOG("AddEntity: " + entityName + " (ID: " + std::to_string(entityId) + ")");
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	QFE_LOG("AddEntity Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
	return entityId;
}

uint32_t SceneObject::RunTimeAddEntity(const std::string& entityName) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	uint32_t entityId = AddEntity(entityName,true);

	if (entityManager_.HasComponent<CsharpComponent>(entityId) && isRunningScript_) {
		CsharpComponent& csharpComponent = entityManager_.GetComponent<CsharpComponent>(entityId);
		for (const auto& ch : csharpComponent.csharpHandles_) {
			csharpScriptExecutor_.CreateScriptInstance(entityId, ch.className_);
		}
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	QFE_LOG("RunTimeAddEntity Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
	return entityId;
}

void SceneObject::DeleteEntity(uint32_t entityId) {
	frameStartCommandInvoker_.AddSystemCommand(
		std::make_unique<DeleteSceneEntityCommand>(entityManager_,csharpScriptExecutor_, entityId));
}

void SceneObject::CopyEntity(uint32_t sourceEntityId) {
	if (!entityManager_.IsActiveEntity(sourceEntityId)) {
		assert(false && "Entity is not active");
		QFE_LOG("CopyEntity failed: Entity " + std::to_string(sourceEntityId) + " is not active", LogLevel::Error);
		return;
	}
	nlohmann::json entityJson;
	SerializeEntity(sourceEntityId, entityJson);
	uint32_t newEntityId = entityManager_.CreateEntity();
	DeserializeEntity(newEntityId, entityJson);
}

void SceneObject::ChangeEntityModel(uint32_t entityId, const std::string& modelName) {
	AssetManager* assetManager = AssetManager::GetInstance();

	// EntityがModelRenderDataを持っていない場合は処理しない
	if (!entityManager_.HasComponent<ModelHandle>(entityId)) {
		QFE_LOG("ChangeModel entity does not have ModelRenderData", LogLevel::Warning);
		return;
	}

	ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId);
	modelHandle.modelName = modelName;
	modelHandle.handle = assetManager->LoadModel(modelName);
}

void SceneObject::ChangeEntityMesh(uint32_t entityId, const std::string& meshName) {
	AssetManager* assetManager = AssetManager::GetInstance();

	//　EntityがModelRenderDataを持っていない、もしくはMeshを持っていない場合は処理しない
	if (!entityManager_.HasComponent<ModelHandle>(entityId)) {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("ChangeMesh entity does not have ModelRenderData", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId);
	ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
	//　Meshがない場合は処理しない
	if (modelData->meshRenderDataHandles.size() == 0) {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("ChangeMesh model does not have mesh", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	modelData->meshRenderDataHandles[0].vertexBufferHandle = assetManager_->LoadModelMesh(meshName);
}

void SceneObject::SaveEntity(uint32_t entityId, const std::string& entityFileName) {

	AssetManager* assetManager = AssetManager::GetInstance();
	if (!entityManager_.IsActiveEntity(entityId)) {
		assert(false && "Entity is not active");
		return;
	}
	nlohmann::json entityJson;
	SerializeEntity(entityId, entityJson);
	std::string entityFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	std::ofstream ofs(entityFilePath + entityFileName + ".json");
	ofs << entityJson.dump(4);
	ofs.close();
}

void SceneObject::ParentChild(uint32_t parentId, uint32_t childId) {
	if (!entityManager_.IsActiveEntity(parentId) || !entityManager_.IsActiveEntity(childId)) {
		assert(false && "Entity is not active");
		return;
	}
	if (!entityManager_.HasComponent<SceneObjectData>(parentId) || !entityManager_.HasComponent<SceneObjectData>(childId)) {
		assert(false && "Entity does not have SceneObjectData");
		return;
	}
	SceneObjectData& parentSceneObjectData = entityManager_.GetComponent<SceneObjectData>(parentId);
	SceneObjectData& childSceneObjectData = entityManager_.GetComponent<SceneObjectData>(childId);
	if (parentSceneObjectData.uniqueId == 0 || childSceneObjectData.uniqueId == 0) {
		assert(false && "Entity does not have uniqueId");
		return;
	}
	if (!entityManager_.HasComponent<ParentData>(childId)) {
		ParentData parentData;
		parentData.parentId = parentSceneObjectData.uniqueId;
		entityManager_.EmplaceComponent<ParentData>(childId, parentData);
	} else {
		ParentData& parentData = entityManager_.GetComponent<ParentData>(childId);
		parentData.parentId = parentSceneObjectData.uniqueId;
	}

	if (!entityManager_.HasComponent<TransformComponent>(parentId) || !entityManager_.HasComponent<TransformComponent>(childId)) {
		return;
	}
	Transform& parentTransform = entityManager_.GetComponent<TransformComponent>(parentId).transform;
	Transform& childTransform = entityManager_.GetComponent<TransformComponent>(childId).transform;
	childTransform.translate -= parentTransform.translate;
}

void SceneObject::Unparent(uint32_t childId) {
	if (!entityManager_.HasComponent<ParentData>(childId)) {
		return;
	}
	entityManager_.RemoveComponent<ParentData>(childId);
}

void SceneObject::SerializeEntity(uint32_t entityId, nlohmann::json& entityJson) {

	entityJson = entityManager_.SerializeEntityComponents(entityId);
}

void SceneObject::DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	usedEntityId_.insert(entityId);

	if (entityJson.contains("SpriteData")) {
		std::chrono::steady_clock::time_point spriteStart = std::chrono::steady_clock::now();
		SpriteData spriteData;
		spriteData.Deserialize(entityJson["SpriteData"]);
		AddSprite(spriteData.textureName, spriteData.width, spriteData.height, static_cast<int>(entityId), static_cast<int>(spriteData.layer), spriteData.pivot);
		std::chrono::steady_clock::time_point spriteEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize SpriteData Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(spriteEnd - spriteStart).count()) + " ms");
	}
	if (entityJson.contains("Transform")) {
		std::chrono::steady_clock::time_point transformStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<TransformComponent>(entityId);
		TransformComponent& transform = entityManager_.GetComponent<TransformComponent>(entityId);
		transform.Deserialize(entityJson["Transform"]);
		std::chrono::steady_clock::time_point transformEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize Transform Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(transformEnd - transformStart).count()) + " ms");
	}
	if (entityJson.contains("ParentData")) {
		std::chrono::steady_clock::time_point parentDataStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<ParentData>(entityId);
		ParentData& parentData = entityManager_.GetComponent<ParentData>(entityId);
		parentData.Deserialize(entityJson["ParentData"]);
		std::chrono::steady_clock::time_point parentDataEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize ParentData Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(parentDataEnd - parentDataStart).count()) + " ms");
	}
	if (entityJson.contains("CameraData")) {
		std::chrono::steady_clock::time_point cameraDataStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<CameraData>(entityId);
		CameraData& cameraData = entityManager_.GetComponent<CameraData>(entityId);
		cameraData.Deserialize(entityJson["CameraData"]);
		std::chrono::steady_clock::time_point cameraDataEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize CameraData Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(cameraDataEnd - cameraDataStart).count()) + " ms");
	}
	if (entityJson.contains("BillboardComponent")) {
		std::chrono::steady_clock::time_point billboardStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<Component::BillboardComponent>(entityId);
		Component::BillboardComponent& billboardComponent = entityManager_.GetComponent<Component::BillboardComponent>(entityId);
		billboardComponent.Deserialize(entityJson["BillboardComponent"]);
		std::chrono::steady_clock::time_point billboardEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize BillboardComponent Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(billboardEnd - billboardStart).count()) + " ms");
	}
	if (entityJson.contains("ModelHandle")) {
		std::chrono::steady_clock::time_point modelHandleStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<ModelHandle>(entityId);
		ModelHandle& modelHandle = entityManager_.GetComponent<ModelHandle>(entityId);
		modelHandle.Deserialize(entityJson["ModelHandle"]);
		std::chrono::steady_clock::time_point modelHandleEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize ModelHandle Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(modelHandleEnd - modelHandleStart).count()) + " ms");
	}
	if (entityJson.contains("SkyboxComponent")) {
		std::chrono::steady_clock::time_point skyboxStart = std::chrono::steady_clock::now();

		AssetManager* assetManager = AssetManager::GetInstance();
		SkyboxComponent skyboxComponent;
		skyboxComponent.textureName = entityJson["SkyboxComponent"]["textureName"].get<std::string>();
		skyboxComponent.textureHandle = assetManager->LoadTexture(skyboxComponent.textureName);
		skyboxComponent.materialBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<Material>();
		skyboxComponent.wvpBufferHandle = assetManager->GetGpuBufferPool()->AcquireConstantBuffer<TransformationMatrix>();
		skyboxComponent.vertexBufferHandle = 
			assetManager->GetModelVertexResourceManager()->AssignBox(DirectXCommon::GetInstance()->GetDevice(), true);

		Material* material = assetManager->GetGpuBufferPool()->GetConstantBufferData<Material>(skyboxComponent.materialBufferHandle);
		material->color = { 1.0f,1.0f,1.0f,1.0f };
		material->enableLighting = false;

		TransformationMatrix* transformData = assetManager->GetGpuBufferPool()->GetConstantBufferData<TransformationMatrix>(skyboxComponent.wvpBufferHandle);
		transformData->World = QFE::Matrix4x4::MakeIndentity4x4();
		transformData->WVP = QFE::Matrix4x4::MakeIndentity4x4();

		entityManager_.EmplaceComponent<SkyboxComponent>(entityId, skyboxComponent);

		std::chrono::steady_clock::time_point skyboxEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize SkyboxComponent Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(skyboxEnd - skyboxStart).count()) + " ms");
	}
	if (entityJson.contains("SceneObjectData")) {
		std::chrono::steady_clock::time_point sceneObjectDataStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<SceneObjectData>(entityId);
		SceneObjectData& sceneObjectData = entityManager_.GetComponent<SceneObjectData>(entityId);
		sceneObjectData.Deserialize(entityJson["SceneObjectData"]);
		uniqueIdManager_.AddUsedID(sceneObjectData.uniqueId);
		std::chrono::steady_clock::time_point sceneObjectDataEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize SceneObjectData Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(sceneObjectDataEnd - sceneObjectDataStart).count()) + " ms");
	}
	if (entityJson.contains("Force")) {
		std::chrono::steady_clock::time_point forceStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<Force>(entityId);
		Force& force = entityManager_.GetComponent<Force>(entityId);
		force.Deserialize(entityJson["Force"]);
		std::chrono::steady_clock::time_point forceEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize Force Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(forceEnd - forceStart).count()) + " ms");
	}
	if (entityJson.contains("SphereColliderData")) {
		std::chrono::steady_clock::time_point sphereColliderStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<SphereColliderData>(entityId);
		SphereColliderData& sphereColliderData = entityManager_.GetComponent<SphereColliderData>(entityId);
		sphereColliderData.Deserialize(entityJson["SphereColliderData"]);
		std::chrono::steady_clock::time_point sphereColliderEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize SphereColliderData Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(sphereColliderEnd - sphereColliderStart).count()) + " ms");
	}
	if (entityJson.contains("AABBColliderData")) {
		std::chrono::steady_clock::time_point aabbColliderStart = std::chrono::steady_clock::now();
		entityManager_.EmplaceComponent<AABBColliderData>(entityId);
		AABBColliderData& aabbColliderData = entityManager_.GetComponent<AABBColliderData>(entityId);
		aabbColliderData.Deserialize(entityJson["AABBColliderData"]);
		std::chrono::steady_clock::time_point aabbColliderEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize AABBColliderData Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(aabbColliderEnd - aabbColliderStart).count()) + " ms");
	}
	if (entityJson.contains("CsharpComponent")) {
		std::chrono::steady_clock::time_point csharpComponentStart = std::chrono::steady_clock::now();
		std::vector<std::string> classNames;
		if (entityJson["CsharpComponent"].contains("CsharpHandles")) {
			//　CsharpHandleからクラス名を取得してスクリプトを追加
			for (const auto& handle : entityJson["CsharpComponent"]["CsharpHandles"]) {
				if (handle.contains("ClassName")) {
					QFE_LOG("Load Csharp Script: " + handle["ClassName"].get<std::string>());
					AddCsharpScript(entityId, handle["ClassName"].get<std::string>());
				}
			}
		}
		std::chrono::steady_clock::time_point csharpComponentEnd = std::chrono::steady_clock::now();
		QFE_LOG("Deserialize CsharpComponent Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(csharpComponentEnd - csharpComponentStart).count()) + " ms");
	}

	QFE_LOG("DeserializeEntity: Entity ID " + std::to_string(entityId) + " deserialized.");
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	QFE_LOG("DeserializeEntity Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
}

uint32_t SceneObject::GetEntityByName(const std::string& entityName) const {
	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<SceneObjectData>(entityId)) {
			const SceneObjectData& sceneObjectData = entityManager_.GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.name == entityName) {
				QFE_LOG("GetEntityByName: Entity with name \"" + entityName + "\" found. Entity ID: " + std::to_string(entityId));
				return entityId;
			}
		}
	}
	assert(false && "Entity Not Found");
	QFE_LOG("GetEntityByName: Entity with name \"" + entityName + "\" not found.", LogLevel::Error);
	return UINT32_MAX;
}

uint32_t SceneObject::GetEntityByUniqueID(uint32_t uniqueId) const {

	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<SceneObjectData>(entityId)) {
			const SceneObjectData& sceneObjectData = entityManager_.GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.uniqueId == uniqueId) {
				return entityId;
			}
		}
	}
	assert(false && "Entity Not Found");
	return 0;
}

std::string QFE::SceneObject::CheckUniqueEntityName(const std::string& baseName) const
{
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

	QFE_LOG("CheckUniqueEntityName: " + baseName);
	std::unordered_set<std::string> existingNames;
	std::vector<uint32_t> entities = entityManager_.GetActiveEntityIds();

	// 既存のエンティティ名をセットに収集
	for (auto entityId : entities) {
		if (entityManager_.HasComponent<SceneObjectData>(entityId)) {
			const SceneObjectData& sceneObjectData = entityManager_.GetComponent<SceneObjectData>(entityId);
			existingNames.insert(sceneObjectData.name);
			QFE_LOG("Existing entity name: " + sceneObjectData.name);
		}
	}

	// baseNameから末尾の " (数字)" を抽出して分離する
	std::string prefix = baseName;
	int counter = 0;

	// ')' で終わり、かつ ' (' が存在するかチェック
	if (baseName.size() > 3 && baseName.back() == ')') {
		size_t openParen = baseName.find_last_of('(');
		if (openParen != std::string::npos && openParen > 0 && baseName[openParen - 1] == ' ') {
			// カッコの中身がすべて数字かチェック
			std::string numStr = baseName.substr(openParen + 1, baseName.size() - openParen - 2);
			bool isNumber = !numStr.empty() && std::all_of(numStr.begin(), numStr.end(), ::isdigit);

			if (isNumber) {
				prefix = baseName.substr(0, openParen - 1);
				counter = std::stoi(numStr);
			}
		}
	}

	// 抽出されたprefixに対して、重複しない名前を探す
	std::string uniqueName = baseName;

	// 初回の counter == 0 (サフィックスなし) かつ、被りがない場合はそのまま
	if (counter == 0 && existingNames.find(uniqueName) == existingNames.end()) {
		return uniqueName;
	}

	// 被りがある、もしくはすでに連番が付いていた場合はループで探す
	// 既に被っている場合は counter を 1 または次の数から開始
	if (counter == 0) counter = 1;

	while (existingNames.find(uniqueName) != existingNames.end()) {
		uniqueName = prefix + " (" + std::to_string(counter) + ")";
		counter++;
	}

	QFE_LOG("Unique name found: " + uniqueName);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	QFE_LOG("CheckUniqueEntityName Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
	return uniqueName;
}
