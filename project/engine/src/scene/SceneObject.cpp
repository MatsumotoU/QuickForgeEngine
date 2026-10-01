#include "engine/include/scene/SceneObject.h"
#include "engine/include/assets/AssetManager.h"
#include <cassert>

#include "engine/include/core/TimeManager.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"
#include "engine/include/assets/Script/LuaScriptResourceManager.h"
#include "engine/include/collider/ColliderManager.h"
#include "engine/include/audio/AudioInterface.h"

#include "engine/include/assets/3DModel/Data/ModelHandle.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/assets/Script/Data/ScriptHandle.h"
#include "engine/include/physics/PhysicsManager.h"
#include "engine/include/collider/Data/SphereColliderData.h"
#include "engine/include/core/Math/ParentData.h"
#include "engine/include/camera/Data/CameraData.h"

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

#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include "Engine/include/scene/SceneCommand/SceneEntityCommands.h"
#include "engine/include/scene/SceneObject.h"

SceneObject::SceneObject() {
	assetManager_ = nullptr;
	isRequestedExit_ = false;
	sceneName_ = "NewScene";
}

SceneObject::~SceneObject() {
}

void SceneObject::Initialize() {
	assetManager_ = AssetManager::GetInstance();
	assert(assetManager_);
	isRequestedExit_ = false;

	CameraManager::GetInstance()->Initialize();
	isRequestStopScript_ = false;
	isRunningScript_ = false;
	isPauseScript_ = false;

	// コマンドクリア
	frameStartCommandInvoker_.ClearCommands();
	updateCommandInvoker_.ClearCommands();
	preDrawCommandInvoker_.ClearCommands();
	drawCommandInvoker_.ClearCommands();
	postDrawCommandInvoker_.ClearCommands();
}

void SceneObject::Update() {
	frameStartCommandInvoker_.ExecuteCommands();

	// ランタイム中のサブモジュールの更新
	if (isRunningScript_ && !isPauseScript_) {
		LuaScriptResourceManager::GetInstance()->UpdateAllScripts();
		PhysicsManager::GetInstance()->Update();
	}

	// 当たり判定更新
	ColliderManager::GetInstance()->Update();

	if (isRunningScript_ && !isPauseScript_) {
		EntityManager* entityManager = assetManager_->GetEntityManager();
		for (uint32_t entityId : entityManager->GetActiveEntityIds()) {
			if (!entityManager->HasComponent<ParticleComponent>(entityId)) {
				continue;
			}
			ParticleComponent& particle = entityManager->GetComponent<ParticleComponent>(entityId);
			ParticleForGPU* gpuData = assetManager_->GetParticleGpuDataManager()->GetDataPtr(particle.particleGpuBufferHandle);
			particle.Update(TimeManager::GetInstance()->GetDeltaTime(), gpuData);
		}
	}

	//　ワールド行列更新
	updateCommandInvoker_.AddCommand(std::make_unique<RemakeUniqeIDCommand>(*(assetManager_->GetEntityManager()),uniqueIdManager_));
	updateCommandInvoker_.AddCommand(std::make_unique<WorldTransformationCommand>(*(assetManager_->GetEntityManager())));
	updateCommandInvoker_.AddCommand(std::make_unique<ParentUpdateCommand>(*(assetManager_->GetEntityManager())));
	updateCommandInvoker_.AddCommand(std::make_unique<AllSpriteResizeCommand>(*(assetManager_->GetEntityManager())));

	// 更新後コマンド実行
	updateCommandInvoker_.ExecuteCommands();
}

void SceneObject::PreDraw() {
	// カメラ更新
	AssetManager* assetManager = AssetManager::GetInstance();
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();
	
	// WVP行列更新
	preDrawCommandInvoker_.AddCommand(std::make_unique<WvpTransformationCommand>(*(assetManager->GetEntityManager()), *cameraManager));
	preDrawCommandInvoker_.AddCommand(std::make_unique<SpritePivotUpdateCommand>(*(assetManager->GetEntityManager())));

	// 描画前コマンド実行
	preDrawCommandInvoker_.ExecuteCommands();
}

void SceneObject::Draw() {
	// 当たり判定の描画
	ColliderManager::GetInstance()->Draw();

	AssetManager* assetManager = AssetManager::GetInstance();
	drawCommandInvoker_.AddCommand(std::make_unique<AllEntityRenderingCommand>(*(assetManager->GetEntityManager())));


	// 描画コマンド実行
	drawCommandInvoker_.ExecuteCommands();
}

void SceneObject::PostDraw() {
	// 描画後コマンド実行
	postDrawCommandInvoker_.ExecuteCommands();
}

void SceneObject::EndFrame() {
	if (isRequestStopScript_) {
		if (isRunningScript_) {
			isRunningScript_ = false;
			LoadScene(sceneName_);
		}
		isRequestStopScript_ = false;
	}
}

void SceneObject::Finalize() {
	
}

void SceneObject::LoadScene(const std::string& sceneName) {
	std::string sceneNameCopy = sceneName;
	// 拡張子確認
	if (!sceneNameCopy.ends_with(".json")) {
		sceneNameCopy += ".json";
	}

#ifdef _DEBUG
	DebugLog("LoadScene: " + sceneNameCopy);
#endif // _DEBUG
	AssetManager* assetManager = AssetManager::GetInstance();
	assetManager->GetGpuBufferPool()->ReleaseAllConstantBuffers();
	assetManager->GetParticleGpuDataManager()->Reset();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityManager->ResetEntiry();
	LuaScriptResourceManager::GetInstance()->Reset();
	AudioInterface::GetInstance()->StopAllSound();

	// シーンファイルを開く
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ifstream ifs(sceneFilePath + sceneNameCopy);
	if (!ifs.is_open()) {
#ifdef _DEBUG
		DebugLog("Faild load scene: " + sceneNameCopy,LogLevel::Error);
#endif // _DEBUG
		CameraManager::GetInstance()->Initialize();
		return;
	}

	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();
	// シーン名の取得
	if (sceneJson.contains("sceneName")) {
		sceneName_ = sceneJson["sceneName"].get<std::string>();
	} else {
		sceneName_ = "NoNameScene";
	}

	// エンティティの生成
	if (!sceneJson.contains("entities")) return;

	for (const auto& entityJson : sceneJson["entities"]) {
		uint32_t entityId = entityManager->CreateEntity();
		DeserializeEntity(entityId, entityJson);
	}

}

void SceneObject::SaveScene(const std::string& sceneName) {
#ifdef _DEBUG
	DebugLog("SaveScene: " + sceneName);
#endif // _DEBUG
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();

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

void SceneObject::ResetScene() {
	uniqueIdManager_.Reset();
}

void SceneObject::RunScene() {

	if (!isRunningScript_) {
#ifdef _DEBUG
		MyDebugLog::GetInstance()->DebugLogClear();
		MyDebugLog::GetInstance()->scriptLogs_.clear();
#endif // _DEBUG

		SaveScene(sceneName_);
		LoadScene(sceneName_);
		isRunningScript_ = true;
		LuaScriptResourceManager::GetInstance()->InitializeAllScripts();
		ColliderManager::GetInstance()->isRunning = true;
		LuaScriptResourceManager::GetInstance()->isRunningScript_ = true;
	}
}

void SceneObject::PauseScene() {
	isPauseScript_ = true;
}

void SceneObject::ResumeScene() {
	isPauseScript_ = false;
}

void SceneObject::StopScene() {
	if (isRequestStopScript_) { return; }
	isRequestStopScript_ = true;
	ColliderManager::GetInstance()->isRunning = false;
	LuaScriptResourceManager::GetInstance()->isRunningScript_ = false;
}

void SceneObject::AddEmptyObject() {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = "EmptyObject";
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddParticleEmitter(const std::string& modelName, uint32_t maxCount) {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	// ParticleComponent追加
	ParticleComponent particleComponent;
	particleComponent.modelName = modelName;
	particleComponent.maxParticleCount = maxCount;
	particleComponent.InitializeResources(*assetManager);
	assetManager->GetEntityManager()->EmplaceComponent<ParticleComponent>(entityId, particleComponent);

	// 既存のものへ追加
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = modelName + "_ParticleEmitter";
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddModel(const std::string& modelName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	ModelHandle modelHandle;
	modelHandle.modelName = modelName;
	modelHandle.handle = assetManager->LoadModel(modelName);
	assetManager->GetEntityManager()->EmplaceComponent<ModelHandle>(entityId, modelHandle);
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = modelName;
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddSprite(const std::string& spriteName, float width, float height, int inEntityId, int layer, Vector2 pivot) {
	AssetManager* assetManager = AssetManager::GetInstance();
	// entityId指定があればそれを使いるなければ新規作成
	uint32_t entityId;
	if (inEntityId != -1) {
		entityId = static_cast<uint32_t>(inEntityId);
	} else {
		entityId = assetManager->GetEntityManager()->CreateEntity();
	}
	// SpriteData追加
	SpriteData spriteData;
	EntityManager* entityManager = assetManager->GetEntityManager();
	spriteData.layer = 0;
	spriteData.pivot = pivot;
	if (entityManager->HasComponentStrage<SpriteData>()) {
		spriteData.layer = static_cast<uint32_t>(entityManager->GetComponentStrage<SpriteData>().size());
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
	// スプライトデータをエンティティに追加
	assetManager->GetEntityManager()->EmplaceComponent<SpriteData>(entityId, spriteData);

	// 既存のものへ追加
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = spriteName;
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneObject::AddLuaScript(uint32_t entityId, const std::string& scriptName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
		ScriptHandles scriptHandles;
		LuaHandle scriptHandle;
		scriptHandle.scriptName_ = scriptName;
		scriptHandle.handle_ = LuaScriptResourceManager::GetInstance()->AddScript(entityId, scriptName);
		LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(scriptHandle.handle_);
		for (std::string& val : script->GetGlobalValuesList()) {
			sol::object obj = script->GetEnvironment()[val];
			if (obj.is<int>()) {
				int v = obj.as<int>();
				scriptHandle.intParams_[val] = v;
			} else if (obj.is<float>()) {
				float v = obj.as<float>();
				scriptHandle.floatParams_[val] = v;
			} else if (obj.is<bool>()) {
				bool v = obj.as<bool>();
				scriptHandle.boolParams_[val] = v;
			} else if (obj.is<std::string>()) {
				std::string v = obj.as<std::string>();
				scriptHandle.stringParams_[val] = v;
			}
		}
		scriptHandles.scriptHandles_.push_back(scriptHandle);
		entityManager->EmplaceComponent<ScriptHandles>(entityId, scriptHandles);
	} else {
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
		// すでに同じスクリプトがアタッチされている場合は追加しない
		for (const auto& sh : scriptHandles.scriptHandles_) {
			if (sh.scriptName_ == scriptName) {
				return;
			}
		}
		LuaHandle scriptHandle;
		scriptHandle.scriptName_ = scriptName;
		scriptHandle.handle_ = LuaScriptResourceManager::GetInstance()->AddScript(entityId, scriptName);
		scriptHandles.scriptHandles_.push_back(scriptHandle);
	}
}

uint32_t SceneObject::AddEntity(const std::string& entityName) {
	AssetManager* assetManager = AssetManager::GetInstance();
#ifdef _DEBUG
	DebugLog("AddEntity: " + entityName);
#endif // _DEBUG

	// 既に読み込んだことがあるエンティティ名ならそれを返す
#ifdef _NODEBUG
	if (loadEntities_.find(entityName) != loadEntities_.end()) {
		// Entityの生成
		uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
		DeserializeEntity(entityId, loadEntities_[entityName]);
		return entityId;
	}
#endif // _NODEBUG

	// Entityのパスを組み立て
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	std::ifstream ifs(sceneFilePath + entityName);
	if (!ifs.is_open()) {
		std::string errorMsg = "FaildOpenFile: " + sceneFilePath + entityName;
#ifdef _DEBUG
		DebugLog(errorMsg, LogLevel::Error);
#endif // _DEBUG
		assert(false && "Faild Open Entity File.");
	}
	// Entityの復元
	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();

	// Entityの生成
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	DeserializeEntity(entityId, sceneJson);
	return entityId;
}

uint32_t SceneObject::RunTimeAddEntity(const std::string& entityName) {
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
	uint32_t entityId = AddEntity(entityName);
	// スクリプト初期化
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(entityId) && isRunningScript_) {
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
		for (const auto& sh : scriptHandles.scriptHandles_) {
			LuaScriptResourceManager::GetInstance()->InitializeScript(sh.handle_);
		}
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
#ifdef _DEBUG
	DebugLog("RunTimeAddEntity Time: " + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()) + " ms");
#endif // _DEBUG
	return entityId;
}

void SceneObject::DeleteEntity(uint32_t entityId)
{
	frameStartCommandInvoker_.AddCommand(std::make_unique<DeleteSceneEntityCommand>(*(assetManager_->GetEntityManager()), entityId));
}

void SceneObject::CopyEntity(uint32_t sourceEntityId) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->IsActiveEntity(sourceEntityId)) {
		assert(false && "Entity is not active");
		return;
	}
	nlohmann::json entityJson;
	SerializeEntity(sourceEntityId, entityJson);
	uint32_t newEntityId = entityManager->CreateEntity();
	DeserializeEntity(newEntityId, entityJson);
}

void SceneObject::ChangeEntityModel(uint32_t entityId, const std::string& modelName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	// エンティティがモデルを持っていなければ何もしない
	if (!entityManager->HasComponent<ModelHandle>(entityId)) {
#ifdef _DEBUG
		DebugLog("ChangeModel entity does not have ModelRenderData", LogLevel::Warning);
#endif // _DEBUG
		return;
	}

	ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
	modelHandle.modelName = modelName;
	modelHandle.handle = assetManager->LoadModel(modelName);
}

void SceneObject::ChangeEntityMesh(uint32_t entityId, const std::string& meshName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	// エンティティがモデルを持っていなければ何もしない
	if (!entityManager->HasComponent<ModelHandle>(entityId)) {
#ifdef _DEBUG
		DebugLog("ChangeMesh entity does not have ModelRenderData", LogLevel::Warning);
#endif // _DEBUG
		return;
	}
	ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
	ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
	// メッシュが存在しなければ何もしない
	if (modelData->meshRenderDataHandles.size() == 0) {
#ifdef _DEBUG
		DebugLog("ChangeMesh model does not have mesh", LogLevel::Warning);
#endif // _DEBUG
		return;
	}
	modelData->meshRenderDataHandles[0].vertexBufferHandle = assetManager_->LoadModelMesh(meshName);
}

void SceneObject::SaveEntity(uint32_t entityId, const std::string& entityFileName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->IsActiveEntity(entityId)) {
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
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->IsActiveEntity(parentId) || !entityManager->IsActiveEntity(childId)) {
		assert(false && "Entity is not active");
		return;
	}
	if (!entityManager->HasComponent<SceneObjectData>(parentId) || !entityManager->HasComponent<SceneObjectData>(childId)) {
		assert(false && "Entity does not have SceneObjectData");
		return;
	}
	SceneObjectData& parentSceneObjectData = entityManager->GetComponent<SceneObjectData>(parentId);
	SceneObjectData& childSceneObjectData = entityManager->GetComponent<SceneObjectData>(childId);
	if (parentSceneObjectData.uniqueId == 0 || childSceneObjectData.uniqueId == 0) {
		assert(false && "Entity does not have uniqueId");
		return;
	}
	if (!entityManager->HasComponent<ParentData>(childId)) {
		ParentData parentData;
		parentData.parentId = parentSceneObjectData.uniqueId;
		entityManager->EmplaceComponent<ParentData>(childId, parentData);
	} else {
		ParentData& parentData = entityManager->GetComponent<ParentData>(childId);
		parentData.parentId = parentSceneObjectData.uniqueId;
	}

	if (!entityManager->HasComponent<Transform>(parentId) || !entityManager->HasComponent<Transform>(childId)) {
		return;
	}
	Transform& parentTransform = entityManager->GetComponent<Transform>(parentId);
	Transform& childTransform = entityManager->GetComponent<Transform>(childId);
	childTransform.translate -= parentTransform.translate;
}

void SceneObject::Unparent(uint32_t childId) {
	if (!AssetManager::GetInstance()->GetEntityManager()->HasComponent<ParentData>(childId)) {
		return;
	}

	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	ParentData& parentData = entityManager->GetComponent<ParentData>(childId);
	uint32_t parentId = 0;
	bool isFound = false;
	if (entityManager->HasComponentStrage<SceneObjectData>()) {
		auto& strage = entityManager->GetComponentStrage<SceneObjectData>();
		for (const auto& [id, sceneObjData] : strage) {
			if (sceneObjData.uniqueId == parentData.parentId) {
				parentId = id;
				isFound = true;
				break;
			}
		}
	}
	if (!isFound) { return; }
	if (!entityManager->HasComponent<Transform>(parentId) || !entityManager->HasComponent<Transform>(childId)) {
		entityManager->RemoveComponent<ParentData>(childId);
		return;
	}
	//Transform& parentTransform = entityManager->GetComponent<Transform>(parentId);
	//Transform& childTransform = entityManager->GetComponent<Transform>(childId);
	// childTransform.translate += parentTransform.translate;

	entityManager->RemoveComponent<ParentData>(childId);
}

void SceneObject::SerializeEntity(uint32_t entityId, nlohmann::json& entityJson) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityJson = entityManager->SerializeEntityComponents(entityId);
}

void SceneObject::DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	usedEntityId_.insert(entityId);

	// 各コンポーネントの復元
	if (entityJson.contains("SpriteData")) {
		SpriteData spriteData;
		spriteData.Deserialize(entityJson["SpriteData"]);
		AddSprite(spriteData.textureName, spriteData.width, spriteData.height, static_cast<int>(entityId), static_cast<int>(spriteData.layer), spriteData.pivot);
	}
	if (entityJson.contains("Transform")) {
		entityManager->EmplaceComponent<Transform>(entityId);
		Transform& transform = entityManager->GetComponent<Transform>(entityId);
		transform.Deserialize(entityJson["Transform"]);
	}
	if (entityJson.contains("ParentData")) {
		entityManager->EmplaceComponent<ParentData>(entityId);
		ParentData& parentData = entityManager->GetComponent<ParentData>(entityId);
		parentData.Deserialize(entityJson["ParentData"]);
	}
	if (entityJson.contains("CameraData")) {
		entityManager->EmplaceComponent<CameraData>(entityId);
		CameraData& cameraData = entityManager->GetComponent<CameraData>(entityId);
		cameraData.Deserialize(entityJson["CameraData"]);
	}
	if (entityJson.contains("ModelHandle")) {
		entityManager->EmplaceComponent<ModelHandle>(entityId);
		ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
		modelHandle.Deserialize(entityJson["ModelHandle"]);
	}
	if (entityJson.contains("ParticleComponent")) {
		entityManager->EmplaceComponent<ParticleComponent>(entityId);
		ParticleComponent& particleComponent = entityManager->GetComponent<ParticleComponent>(entityId);
		particleComponent.Deserialize(entityJson["ParticleComponent"]);
		particleComponent.InitializeResources(*assetManager);
	}
	if (entityJson.contains("SceneObjectData")) {
		entityManager->EmplaceComponent<SceneObjectData>(entityId);
		SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
		sceneObjectData.Deserialize(entityJson["SceneObjectData"]);
		uniqueIdManager_.AddUsedID(sceneObjectData.uniqueId);
	}
	if (entityJson.contains("Force")) {
		entityManager->EmplaceComponent<Force>(entityId);
		Force& force = entityManager->GetComponent<Force>(entityId);
		force.Deserialize(entityJson["Force"]);
	}
	if (entityJson.contains("SphereColliderData")) {
		entityManager->EmplaceComponent<SphereColliderData>(entityId);
		SphereColliderData& sphereColliderData = entityManager->GetComponent<SphereColliderData>(entityId);
		sphereColliderData.Deserialize(entityJson["SphereColliderData"]);
	}
	if (entityJson.contains("AABBColliderData")) {
		entityManager->EmplaceComponent<AABBColliderData>(entityId);
		AABBColliderData& aabbColliderData = entityManager->GetComponent<AABBColliderData>(entityId);
		aabbColliderData.Deserialize(entityJson["AABBColliderData"]);
	}
	if (entityJson.contains("ScriptHandle")) {
		std::vector<std::string> scriptNames;
		if (entityJson.contains("ScriptHandle") && entityJson["ScriptHandle"].contains("scriptHandles")) {
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
#ifdef _DEBUG
					DebugLog("Load Script: " + handle["scriptName"].get<std::string>());
#endif // _DEBUG
					AddLuaScript(entityId, handle["scriptName"].get<std::string>());
				}
			}
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
					ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
					std::vector<uint32_t> luaHandles;
					for (auto& sh : scriptHandles.scriptHandles_) {
						luaHandles.push_back(sh.handle_);
					}
					scriptHandles.Deserialize(entityJson["ScriptHandle"]);
					for (size_t i = 0; i < luaHandles.size(); ++i) {
						scriptHandles.scriptHandles_[i].handle_ = luaHandles[i];
					}
					for (LuaHandle& hl : scriptHandles.scriptHandles_) {
						LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(hl.handle_);
						sol::environment& env = script->GetEnvironment();
						for (const auto& [key, val] : hl.intParams_) {
							env[key] = val;
						}
						for (const auto& [key, val] : hl.floatParams_) {
							env[key] = val;
						}
						for (const auto& [key, val] : hl.boolParams_) {
							env[key] = val;
						}
						for (const auto& [key, val] : hl.stringParams_) {
							env[key] = val;
						}

						script->SetPriority(hl.priority_);
					}
				}
			}

		}
	}
}

uint32_t SceneObject::GetEntityByName(const std::string& entityName) const {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			const SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.name == entityName) {
				return entityId;
			}
		}
	}
	assert(false && "Entity Not Found");
	return 0;
}

uint32_t SceneObject::GetEntityByUniqeID(uint32_t uniqueId) const {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			const SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.uniqueId == uniqueId) {
				return entityId;
			}
		}
	}
	assert(false && "Entity Not Found");
	return 0;
}
