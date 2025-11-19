#include "SceneManager.h"
#include "SceneObject.h"

#include "Assets/AssetManager.h"
#include "Camera/CameraManager.h"
#include <fstream>
#include <string>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void SceneManager::Initalize() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

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

	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef _DEBUG
	initTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // _DEBUG
}

void SceneManager::Update() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

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

	currentScene_->Update();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef _DEBUG
	updateTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // _DEBUG
}

void SceneManager::PreDraw() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	currentScene_->PreDraw();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef _DEBUG
	preDrawTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // _DEBUG
}

void SceneManager::Draw() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	currentScene_->Draw();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef _DEBUG
	drawTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // _DEBUG
}

void SceneManager::PostDraw() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	currentScene_->PostDraw();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef _DEBUG
	postDrawTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // _DEBUG
}

void SceneManager::EndFrame() {
	currentScene_->EndFrame();
}

void SceneManager::Finalize() {
	currentScene_->Finalize();
#ifdef _DEBUG
	sceneConfig_["lastScene"] = currentScene_->GetSceneName();
	try {
		std::string path = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Config") + "SceneConfig.json";
		std::ofstream ofs(path);
		ofs << sceneConfig_.dump(4);
		ofs.close();
		DebugLog("SaveSceneConfig");
	}
	catch (const std::exception& e) {
		DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);

	}
#endif // _DEBUG
	CameraManager::GetInstance()->Shutdown();
}

uint32_t SceneManager::GetEntityByName(const std::string& entityName) const {
	return currentScene_->GetEntityByName(entityName);
}

uint32_t SceneManager::GetEntityByUniqeID(uint32_t uniqueId) const {
	return currentScene_->GetEntityByUniqeID(uniqueId);
}

void SceneManager::SaveScene(const std::string& sceneName) {
	currentScene_->SaveScene(sceneName);
}

void SceneManager::LoadScene(const std::string& sceneName) {
	currentScene_->LoadScene(sceneName);
}

void SceneManager::ResetScene() {
	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();
	AssetManager::GetInstance()->GetEntityManager()->ResetEntiry();
	CameraManager::GetInstance()->Initialize();
}

void SceneManager::RunTimeSwapScene(const std::string& sceneName) {
	StopScript();
	isRequestRunTimeLoadScene_ = true;
	nextSceneName_ = sceneName;
}

void SceneManager::CopyEntity(uint32_t sourceEntityId) {
	currentScene_->CopyEntity(sourceEntityId);
}

void SceneManager::ChangeEntityModel(uint32_t entityId, const std::string& modelName) {
	currentScene_->ChangeEntityModel(entityId, modelName);
	
}

void SceneManager::SaveEntity(uint32_t entityId, const std::string& entityFileName) {
	currentScene_->SaveEntity(entityId, entityFileName);
	
}

void SceneManager::ParentChild(uint32_t parentId, uint32_t childId) {
	currentScene_->ParentChild(parentId, childId);
}

void SceneManager::Unparent(uint32_t childId) {
	currentScene_->Unparent(childId);
}

void SceneManager::SerializeEntity(uint32_t entityId, nlohmann::json& entityJson) {
	currentScene_->SerializeEntity(entityId, entityJson);
}

void SceneManager::DeserializeEntity(uint32_t entityId, const nlohmann::json& entityJson) {
	currentScene_->DeserializeEntity(entityId, entityJson);

}

void SceneManager::AddEpmtyObject() {
	currentScene_->AddEmptyObject();
}

void SceneManager::AddModel(const std::string& modelName) {
	currentScene_->AddModel(modelName);
}

void SceneManager::AddSprite(const std::string& spriteName, float width, float height, int inEntityId, int layer, Vector2 pvot) {
	currentScene_->AddSprite(spriteName, width, height, inEntityId, layer, pvot);
}

void SceneManager::AddLuaScript(uint32_t entityId, const std::string& scriptName) {
	currentScene_->AddLuaScript(entityId, scriptName);
}

void SceneManager::AddCsharpScript(uint32_t entityId, const std::string& className) {
	currentScene_->AddCsharpScript(entityId, className);
}

uint32_t SceneManager::AddEntity(const std::string& entityName) {
	return currentScene_->AddEntity(entityName);
}

uint32_t SceneManager::RunTimeAddEntity(const std::string& entityName) {
	return currentScene_->RunTimeAddEntity(entityName);
}

void SceneManager::StartScript() {
	currentScene_->RunScene();
}

void SceneManager::StopScript() {
	currentScene_->StopScene();
}
