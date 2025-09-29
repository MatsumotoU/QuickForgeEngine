#include "SceneManager.h"
#include "SceneObject.h"

#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include "Assets/Script/LuaScriptResourceManager.h"

#include "Assets/3DModel/Data/ModelHandle.h"
#include "Data/SceneObjectData.h"
#include "Assets/Script/Data/ScriptHandle.h"

#include <fstream>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG


void SceneManager::Initalize() {
	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();

	CameraManager::GetInstance()->Initialize();
}

void SceneManager::Update() {
	currentScene_->Update();
}

void SceneManager::PreDraw() {
	// ワールド行列更新
	AssetManager* assetManager = AssetManager::GetInstance();
	std::vector<uint32_t> entities = assetManager->GetEntityManager()->GetActiveEntityIds();
	for (auto entityId : entities) {
		if (assetManager->GetEntityManager()->HasComponent<Transform>(entityId)) {
			Transform& transform = assetManager->GetEntityManager()->GetComponent<Transform>(entityId);
			if (assetManager->GetEntityManager()->HasComponent<ModelHandle>(entityId)) {
				ModelHandle& modelHandle = assetManager->GetEntityManager()->GetComponent<ModelHandle>(entityId);
				TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(modelHandle.handle);
				wpvMatrix->World = Matrix4x4::MakeAffineMatrix(
					transform.scale,
					transform.rotate,
					transform.translate
				);
			}
		}
	}

	// カメラ更新
	CameraManager* cameraManager = CameraManager::GetInstance();
	cameraManager->Update();
	for (uint32_t i = 0; i < assetManager->GetWpvBufferManager()->GetBufferCount(); i++) {
		Camera& camera = cameraManager->GetMainCamera();
		TransformationMatrix* wpvMatrix = assetManager->GetWpvBufferManager()->GetBufferData(i);
		wpvMatrix->WVP = camera.GetWorldViewProjectionMatrix(wpvMatrix->World);
	}
}

void SceneManager::Draw() {
	currentScene_->Draw();
}

void SceneManager::PostDraw() {
}

void SceneManager::Finalize() {
	CameraManager::GetInstance()->Shutdown();
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
		entityJson = entityManager->SerializeEntityComponents(entityId);
		sceneJson["entities"].push_back(entityJson);
	}

	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ofstream ofs(sceneFilePath + sceneName + ".json");
	ofs << sceneJson.dump(4);
	ofs.close();
}

void SceneManager::LoadScene(const std::string& sceneName) {
#ifdef _DEBUG
	DebugLog("LoadScene: " + sceneName);
#endif // _DEBUG
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	entityManager->ResetEntiry();

	// シーンファイルのパスを組み立て
	std::string sceneFilePath = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scenes");
	std::ifstream ifs(sceneFilePath + sceneName);
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

		// 必要なコンポーネントを追加
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
		if(entityJson.contains("ScriptHandle")) {
			std::vector<std::string> scriptNames;
			if (entityJson.contains("ScriptHandle") && entityJson["ScriptHandle"].contains("scriptHandles")) {
				for (const auto& handle : entityJson["ScriptHandle"]["scriptHandles"]) {
					if (handle.contains("scriptName")) {
						AddScript(entityId, handle["scriptName"].get<std::string>());
					}
				}
			}
		}
	}
}

void SceneManager::ResetScene() {
	AssetManager::GetInstance()->GetEntityManager()->ResetEntiry();
	CameraManager::GetInstance()->Initialize();
}

void SceneManager::LoadModel(const std::string& modelName) {
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

void SceneManager::AddScript(uint32_t entityId, const std::string& scriptName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	EntityManager* entityManager = assetManager->GetEntityManager();
	if (!entityManager->HasComponent<ScriptHandles>(entityId)) {
		ScriptHandles scriptHandles;
		LuaHandle scriptHandle;
		scriptHandle.scriptName_ = scriptName;
		scriptHandle.handle_ = LuaScriptResourceManager::GetInstance()->AddScript(entityId, scriptName);
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
