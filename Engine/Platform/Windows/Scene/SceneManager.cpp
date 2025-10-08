#include "SceneManager.h"
#include "SceneObject.h"

#include "Core/EngineGlobalValue.h"

#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"
#include "Collider/ColliderManager.h"

#include "Assets/3DModel/Data/ModelHandle.h"
#include "Data/SceneObjectData.h"
#include "Assets/Script/Data/ScriptHandle.h"
#include "Physics/PhysicsManager.h"
#include "Collider/Data/SphereColliderData.h"

#include <fstream>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG


void SceneManager::Initalize() {
	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();

	CameraManager::GetInstance()->Initialize();
	isRequestStopScript_ = false;
	isRunningScript_ = false;
}

void SceneManager::Update() {
	// スクリプト更新
	ColliderManager::GetInstance()->Update();
	if (isRunningScript_) {
		LuaScriptResourceManager::GetInstance()->UpdateAllScripts();
		PhysicsManager::GetInstance()->Update();
	}
	currentScene_->Update();
}

void SceneManager::PreDraw() {
	// カメラ更新
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();

	// ワールド行列更新
	AssetManager* assetManager = AssetManager::GetInstance();
	std::vector<uint32_t> entities = assetManager->GetEntityManager()->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (assetManager->GetEntityManager()->HasComponent<Transform>(entityId)) {
			Transform& transform = assetManager->GetEntityManager()->GetComponent<Transform>(entityId);
			// モデルのワールド行列更新
			if (assetManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = assetManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
				const ModelRenderData* modelData = assetManager->GetModelRenderData(modelHandle.handle);
				// メッシュごとにワールド行列更新
				for (const auto& meshData : modelData->meshRenderDataHandles) {
					TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(meshData.wpvBufferHandle);
					wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
						transform.scale,
						transform.rotate,
						transform.translate
					);
					wpvMatrix->WVP = cameraManager->GetMainCamera().GetWorldViewProjectionMatrix(wpvMatrix->World, CameraType::Perspective);
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
	if (entityJson.contains("ModelHandle")) {
		entityManager->EmplaceComponent<ModelHandle>(entityId);
		ModelHandle& modelHandle = entityManager->GetComponent<ModelHandle>(entityId);
		modelHandle.Deserialize(entityJson["ModelHandle"]);
	}
	if (entityJson.contains("SceneObjectData")) {
		entityManager->EmplaceComponent<SceneObjectData>(entityId);
		SceneObjectData& sceneObjectData = entityManager->GetComponent<SceneObjectData>(entityId);
		sceneObjectData.Deserialize(entityJson["SceneObjectData"]);
	}
	if (entityJson.contains("ScriptHandle")) {
		std::vector<std::string> scriptNames;
		if (entityJson.contains("ScriptHandle") && entityJson["ScriptHandle"].contains("scriptHandles")) {
			for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
				if (handle.contains("scriptName")) {
					AddScript(entityId, handle["scriptName"].get<std::string>());
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

					// グローバル変数の復元
					for (LuaHandle& hl : scriptHandles.scriptHandles_) {
						sol::state* state = LuaScriptResourceManager::GetInstance()->GetScript(hl.handle_)->GetScript();
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
					}
				}
			}
		}
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
}

void SceneManager::AddEpmtyObject() {
	AssetManager* assetManager = AssetManager::GetInstance();
	uint32_t entityId = assetManager->GetEntityManager()->CreateEntity();
	assetManager->GetEntityManager()->EmplaceComponent<Transform>(entityId, Transform());
	SceneObjectData sceneObjectData;
	sceneObjectData.name = "EmptyObject";
	sceneObjectData.tag = "Untagged";
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
	}
	if (height != 0.0f) {
		textureSize.y = height;
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
	
	// Entityのパスを組み立て
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Entities");
	std::ifstream ifs(sceneFilePath + entityName);
	if (!ifs.is_open()) {
		assert(false && "FaildOpenFile");
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

void SceneManager::StartScript() {
	if (!isRunningScript_) {
#ifdef _DEBUG
		MyDebugLog::GetInstance()->DebugLogClear();
#endif // _DEBUG

		SaveScene(currentScene_->GetSceneName());
		LoadScene(currentScene_->GetSceneName());
		isRunningScript_ = true;
		LuaScriptResourceManager::GetInstance()->InitializeAllScripts();
		ColliderManager::GetInstance()->isRunning = true;
	}
}

void SceneManager::StopScript() {
	if (isRequestStopScript_) { return; }
	isRequestStopScript_ = true;
	ColliderManager::GetInstance()->isRunning = false;
}
