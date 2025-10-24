#include "SceneManager.h"
#include "SceneObject.h"

#include "Core/EngineGlobalValue.h"

#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Collider/ColliderManager.h"
#include "Audio/AudioInterface.h"

#include "Assets/3DModel/Data/ModelHandle.h"
#include "Data/SceneObjectData.h"
#include "Assets/Script/Data/ScriptHandle.h"
#include "Physics/PhysicsManager.h"
#include "Collider/Data/SphereColliderData.h"
#include "Core/Math/ParentData.h"
#include "Camera/Data/CameraData.h"

#include <fstream>
#include <execution>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG
#include "Assets/3DModel/Loader/AssimpModelLoader.h"


void SceneManager::Initalize() {
	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();

	CameraManager::GetInstance()->Initialize();
	isRequestStopScript_ = false;
	isRunningScript_ = false;
	isRequestRunTimeLoadScene_ = false;
	isFirstLoadScene_ = false;

	// SceneConfig.jsonの読み込み
	sceneConfig_ = nlohmann::json::object();
	try {
		std::string path = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Config") + "SceneConfig.json";
		std::ifstream ifs(path);
		if (ifs.is_open()) {
			ifs >> sceneConfig_;
			ifs.close();
		}
#ifdef _DEBUG
		DebugLog("Load SceneConfig.json");
#endif // _DEBUG
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);
#endif // _DEBUG
	}

	
}

void SceneManager::Update() {
	// 最後に開いたシーンをロード
	if (!isFirstLoadScene_) {
		if (sceneConfig_.contains("lastScene")) {
			try {
				LoadScene(sceneConfig_["lastScene"].get<std::string>());
#ifdef NDEBUG
				StartScript();
#endif // _RELEASE

			}
			catch (const std::exception& e) {
#ifdef _DEBUG
				DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);
#endif // _DEBUG
			}
		}
		isFirstLoadScene_ = true;
	}

	if (isRequestRunTimeLoadScene_) {
		LoadScene(nextSceneName_);
		StartScript();
		isRequestRunTimeLoadScene_ = false;
		nextSceneName_.clear();
	}

	// スクリプト更新
	if (isRunningScript_) {
		LuaScriptResourceManager::GetInstance()->UpdateAllScripts();
		PhysicsManager::GetInstance()->Update();
	}
	currentScene_->Update();

	// コライダー更新
	ColliderManager::GetInstance()->Update();

	// ユニークIDが未設定なら設定する
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	std::vector<uint32_t> entities = entityManager->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (sceneObjectData.uniqueId == 0) {
				sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
			} else {
				uniqueIdManager_.AddUsedID(sceneObjectData.uniqueId);
			}
		}
	}
	// ユニークIDが重複していたら再設定する
	std::set<uint32_t> checkIds;
	for (auto entityId : entities) {
		if (entityManager->HasComponent<SceneObjectData>(entityId)) {
			SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
			if (checkIds.find(sceneObjectData.uniqueId) != checkIds.end()) {
				sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
			}
			checkIds.insert(sceneObjectData.uniqueId);
		}
	}

	// ワールド行列更新(wvpを別コンポーネントにする)
	AssetManager* assetManager = AssetManager::GetInstance();
	for (auto entityId : entities) {
		if (entityManager->HasComponent<Transform>(entityId)) {
			Transform& transform = entityManager->GetComponent<Transform>(entityId);
			// モデルのワールド行列更新
			if (entityManager->HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
				const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
				// メッシュごとにワールド行列更新
				for (const auto& meshData : modelData->meshRenderDataHandles) {
					TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(meshData.wpvBufferHandle);
					wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
						transform.scale,
						transform.rotate,
						transform.translate
					);
				}
			}
			// スプライトのワールド行列更新
			if (assetManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
				SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle);
				wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
					transform.scale,
					transform.rotate,
					transform.translate
				);
			}
		}
	}

	// ペアレント子関係更新
	for (auto entityId : entities) {
		if (assetManager->GetEntityManager()->HasComponent<ParentData>(entityId)) {
			ParentData& parentData = assetManager->GetEntityManager()->GetComponent<ParentData>(entityId);

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
			if (!isFound) { continue; }

			if (assetManager->GetEntityManager()->HasComponent<Transform>(parentId)) {
				Transform& parentTransform = assetManager->GetEntityManager()->GetComponent<Transform>(parentId);
				// モデルのワールド行列更新
				if (assetManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
					ModelHandle& modelHandle = assetManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
					const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
					// メッシュごとにワールド行列更新
					for (const auto& meshData : modelData->meshRenderDataHandles) {
						TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(meshData.wpvBufferHandle);
						wpvMatrix->World = Matrix4x4::Multiply(wpvMatrix->World, Matrix4x4::MakeAffineMatrix(
							parentTransform.scale, parentTransform.rotate, parentTransform.translate));
					}
				}
				// スプライトのワールド行列更新
				if (assetManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
					SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
					TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle);
					wpvMatrix->World = Matrix4x4::Multiply(wpvMatrix->World, Matrix4x4::MakeAffineMatrix(
						parentTransform.scale, parentTransform.rotate, parentTransform.translate));
				}
			}

		}
	}
}

void SceneManager::PreDraw() {
	// カメラ更新
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();

	// ビュー行列更新
	AssetManager* assetManager = AssetManager::GetInstance();
	std::vector<uint32_t> entities = assetManager->GetEntityManager()->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (assetManager->GetEntityManager()->HasComponent<Transform>(entityId)) {
			// モデルのワールド行列更新
			if (assetManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = assetManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
				const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
				// メッシュごとにワールド行列更新
				for (const auto& meshData : modelData->meshRenderDataHandles) {
					TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(meshData.wpvBufferHandle);
					wpvMatrix->WVP = cameraManager->GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Perspective);
				}
			}
			// スプライトのワールド行列更新
			if (assetManager->GetEntityManager()->HasComponent<SpriteData>(entityId)) {
				SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle);
				wpvMatrix->WVP = cameraManager->GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Orthographic);
			}
		}
	}
}

void SceneManager::Draw() {
	ColliderManager::GetInstance()->Draw();
	currentScene_->Draw();
}

void SceneManager::PostDraw() {
}

void SceneManager::EndFrame() {
	if (isRequestStopScript_) {
		if (isRunningScript_) {
			isRunningScript_ = false;
			LoadScene(currentScene_->GetSceneName());
		}
		isRequestStopScript_ = false;
	}
}

void SceneManager::Finalize() {
	sceneConfig_["lastScene"] = currentScene_->GetSceneName();
	try {
		std::string path = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Config") + "SceneConfig.json";
		std::ofstream ofs(path);
		ofs << sceneConfig_.dump(4);
		ofs.close();
#ifdef _DEBUG
		DebugLog("SaveSceneConfig");
#endif // _DEBUG
	}
	catch (const std::exception& e) {
#ifdef _DEBUG
		DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);
#endif // _DEBUG
	}

	CameraManager::GetInstance()->Shutdown();
}

uint32_t SceneManager::GetEntityByName(const std::string& entityName) const {
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

uint32_t SceneManager::GetEntityByUniqeID(uint32_t uniqueId) const {
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

void SceneManager::SaveScene(const std::string& sceneName) {
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

void SceneManager::LoadScene(const std::string& sceneName) {
	std::string sceneNameCopy = sceneName;
	// 拡張子がなければ追加
	if (!sceneNameCopy.ends_with(".json")) {
		sceneNameCopy += ".json";
	}

#ifdef _DEBUG
	DebugLog("LoadScene: " + sceneNameCopy);
#endif // _DEBUG
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityManager->ResetEntiry();
	LuaScriptResourceManager::GetInstance()->Reset();
	AudioInterface::GetInstance()->StopAllSound();

	// シーンファイルのパスを組み立て
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ifstream ifs(sceneFilePath + sceneNameCopy);
	if (!ifs.is_open()) {
		assert(false && "FaildOpenFile");
	}

	nlohmann::json sceneJson;
	ifs >> sceneJson;
	ifs.close();
	// シーン名の設定
	if (sceneJson.contains("sceneName")) {
		currentScene_->SetSceneName(sceneJson["sceneName"].get<std::string>());
	} else {
		currentScene_->SetSceneName("NoNameScene");
	}

	// エンティティの復元
	if (!sceneJson.contains("entities")) return;

	for (const auto& entityJson : sceneJson["entities"]) {
		uint32_t entityId = entityManager->CreateEntity();
		DeserializeEntity(entityId, entityJson);
	}
}

void SceneManager::ResetScene() {
	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();
	AssetManager::GetInstance()->GetEntityManager()->ResetEntiry();
	CameraManager::GetInstance()->Initialize();
	uniqueIdManager_.Reset();
}

void SceneManager::RunTimeSwapScene(const std::string& sceneName) {
	StopScript();
	isRequestRunTimeLoadScene_ = true;
	nextSceneName_ = sceneName;
}

void SceneManager::ChangeEntityModel(uint32_t entityId, const std::string& modelName) {
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

void SceneManager::SaveEntity(uint32_t entityId, const std::string& entityFileName) {
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

void SceneManager::ParentChild(uint32_t parentId, uint32_t childId) {
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

void SceneManager::Unparent(uint32_t childId) {
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

void SceneManager::SerializeEntity(uint32_t entityId, nlohmann::json& entityJson) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityJson = entityManager->SerializeEntityComponents(entityId);
}

void SceneManager::DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();

	// 必要なコンポーネントを追加
	if (entityJson.contains("SpriteData")) {
		SpriteData spriteData;
		spriteData.Deserialize(entityJson["SpriteData"]);
		AddSprite(spriteData.textureName, spriteData.width, spriteData.height, static_cast<int>(entityId), static_cast<int>(spriteData.layer));
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

			// スクリプトの復元
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
#ifdef _DEBUG
					DebugLog("Load Script: " + handle["scriptName"].get<std::string>());
#endif // _DEBUG
					AddScript(entityId, handle["scriptName"].get<std::string>());
				}
			}
			// グローバル変数の復元
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
					ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
					// グローバル変数の復元準備
					std::vector<uint32_t> luaHandles;
					for (auto& sh : scriptHandles.scriptHandles_) {
						luaHandles.push_back(sh.handle_);
					}
					scriptHandles.Deserialize(entityJson["ScriptHandle"]);
					for (size_t i = 0; i < luaHandles.size(); ++i) {
						scriptHandles.scriptHandles_[i].handle_ = luaHandles[i];
					}

					// 後付け情報の復元
					for (LuaHandle& hl : scriptHandles.scriptHandles_) {
						// グローバル変数の復元
						LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(hl.handle_);
						sol::state* state = script->GetScript();
						for (const auto& [key, val] : hl.intParams_) {
							(*state)[key] = val;
						}
						for (const auto& [key, val] : hl.floatParams_) {
							(*state)[key] = val;
						}
						for (const auto& [key, val] : hl.boolParams_) {
							(*state)[key] = val;
						}
						for (const auto& [key, val] : hl.stringParams_) {
							(*state)[key] = val;
						}

						// 優先度の復元
						script->SetPriority(hl.priority_);
					}
				}
			}

		}
	}

}

void SceneManager::AddEpmtyObject() {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = "EmptyObject";
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneManager::AddModel(const std::string& modelName) {
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

void SceneManager::AddSprite(const std::string& spriteName, float width, float height, int inEntityId, int layer) {
	AssetManager* assetManager = AssetManager::GetInstance();
	// entityId指定があればそれを使う、なければ新規作成
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
	spriteData.wvpBufferHandle = assetManager->GetWpvBufferManager()->CreateBuffer();
	assetManager->GetWpvBufferManager()->GetBufferData(spriteData.wvpBufferHandle)->World = Matrix4x4::MakeIndentity4x4();
	spriteData.materialBufferHandle = assetManager->GetMaterialBufferManager()->CreateBuffer();
	Material* material = assetManager->GetMaterialBufferManager()->GetBufferData(spriteData.materialBufferHandle);
	material->color = { 1.0f,1.0f,1.0f,1.0f };
	material->enableLighting = false;
	material->uvTransform = Matrix4x4::MakeIndentity4x4();
	spriteData.lightBufferHandle = assetManager->GetLightBufferManager()->CreateBuffer();
	DirectionalLight* light = assetManager->GetLightBufferManager()->GetBufferData(spriteData.lightBufferHandle);
	light->color = { 1.0f,1.0f,1.0f,1.0f };
	light->direction = { 0.0f,-1.0f,0.0f };
	light->intensity = 1.0f;
	// スプライトデータをエンティティに追加
	assetManager->GetEntityManager()->EmplaceComponent<SpriteData>(entityId, spriteData);

	// いつものやつ追加
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = spriteName;
	sceneObjectData.tag = "Untagged";
	sceneObjectData.uniqueId = uniqueIdManager_.GenerateUniqueID();
	assetManager->GetEntityManager()->EmplaceComponent<SceneObjectData>(entityId, sceneObjectData);
}

void SceneManager::AddScript(uint32_t entityId, const std::string& scriptName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
		ScriptHandles scriptHandles;
		LuaHandle scriptHandle;
		scriptHandle.scriptName_ = scriptName;
		scriptHandle.handle_ = LuaScriptResourceManager::GetInstance()->AddScript(entityId, scriptName);
		LuaScriptOnQFE* script = LuaScriptResourceManager::GetInstance()->GetScript(scriptHandle.handle_);
		for (std::string& val : script->GetGlobalValuesList()) {
			sol::state* state = script->GetScript();
			sol::object obj = (*state)[val];
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

uint32_t SceneManager::AddEntity(const std::string& entityName) {
	AssetManager* assetManager = AssetManager::GetInstance();
#ifdef _DEBUG
	DebugLog("AddEntity: " + entityName);
#endif // _DEBUG

	// 既に読み込んだことがあるエンティティ名ならそれを返す
	if (loadEntities_.find(entityName) != loadEntities_.end()) {
		// Entityの生成
		uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
		DeserializeEntity(entityId, loadEntities_[entityName]);
		return entityId;
	}

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
	// 読み込んだエンティティ名を保存
	loadEntities_[entityName] = sceneJson;

	return entityId;
}

uint32_t SceneManager::RunTimeAddEntity(const std::string& entityName) {
	uint32_t entityId = AddEntity(entityName);
	// スクリプト初期化
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	if (entityManager->HasComponent<ScriptHandles>(entityId) && isRunningScript_) {
		ScriptHandles& scriptHandles = entityManager->GetComponent<ScriptHandles>(entityId);
		for (const auto& sh : scriptHandles.scriptHandles_) {
			LuaScriptResourceManager::GetInstance()->InitializeScript(sh.handle_);
		}
	}
	return entityId;
}

void SceneManager::StartScript() {
	if (!isRunningScript_) {
#ifdef _DEBUG
		MyDebugLog::GetInstance()->DebugLogClear();
		MyDebugLog::GetInstance()->scriptLogs_.clear();
#endif // _DEBUG

		SaveScene(currentScene_->GetSceneName());
		LoadScene(currentScene_->GetSceneName());
		isRunningScript_ = true;
		LuaScriptResourceManager::GetInstance()->InitializeAllScripts();
		ColliderManager::GetInstance()->isRunning = true;
		LuaScriptResourceManager::GetInstance()->isRunningScript_ = true;
	}
}

void SceneManager::StopScript() {
	if (isRequestStopScript_) { return; }
	isRequestStopScript_ = true;
	ColliderManager::GetInstance()->isRunning = false;
	LuaScriptResourceManager::GetInstance()->isRunningScript_ = false;
}
