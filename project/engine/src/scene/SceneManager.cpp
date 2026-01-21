#include "engine/include/scene/SceneManager.h"
#include "engine/include/scene/SceneObject.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/camera/CameraManager.h"
#include <fstream>
#include <string>

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

void SceneManager::Initialize() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	isRequestRunTimeLoadScene_ = false;
	isFirstLoadScene_ = false;

	// SceneConfig.json縺ｮ隱ｭ縺ｿ霎ｼ縺ｿ
	sceneConfig_ = nlohmann::json::object();
	try {
		std::string path = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Config") + "SceneConfig.json";
		std::ifstream ifs(path);
		if (ifs.is_open()) {
			ifs >> sceneConfig_;
			ifs.close();
		}
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Load SceneConfig.json");
#endif // QFE_OPTIMIZE_OFF
	}
	catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);
#endif // QFE_OPTIMIZE_OFF
	}

	currentScene_ = std::make_unique<SceneObject>();
	currentScene_->Initialize();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef QFE_OPTIMIZE_OFF
	initTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // QFE_OPTIMIZE_OFF
	score_ = 0;
}

void SceneManager::Update() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	// 譛蠕後↓髢九＞縺溘す繝ｼ繝ｳ繧帝幕縺・
	if (!isFirstLoadScene_) {
		if (sceneConfig_.contains("lastScene")) {
			try {
				LoadScene(sceneConfig_["lastScene"].get<std::string>());
#ifdef NDEBUG
				StartScript();
#endif // _RELEASE

			}
			catch (const std::exception& e) {
#ifdef QFE_OPTIMIZE_OFF
				DebugLog(std::string("Error: ") + e.what(), LogLevel::EditorInfo);
#endif // QFE_OPTIMIZE_OFF
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
#ifdef QFE_OPTIMIZE_OFF
	updateTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // QFE_OPTIMIZE_OFF
}

void SceneManager::PreDraw() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	currentScene_->PreDraw();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef QFE_OPTIMIZE_OFF
	preDrawTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // QFE_OPTIMIZE_OFF
}

void SceneManager::Draw() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	currentScene_->Draw();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef QFE_OPTIMIZE_OFF
	drawTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // QFE_OPTIMIZE_OFF
}

void SceneManager::PostDraw() {
	std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

	currentScene_->PostDraw();

	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
#ifdef QFE_OPTIMIZE_OFF
	postDrawTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count());
#endif // QFE_OPTIMIZE_OFF
}

void SceneManager::EndFrame() {
	currentScene_->EndFrame();
}

void SceneManager::Finalize() {
	currentScene_->Finalize();
#ifdef QFE_OPTIMIZE_OFF
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
#endif // QFE_OPTIMIZE_OFF
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

void SceneManager::DeleteEntity(uint32_t entityId)
{
	currentScene_->DeleteEntity(entityId);
}

void SceneManager::CopyEntity(uint32_t sourceEntityId) {
	currentScene_->CopyEntity(sourceEntityId);
}

void SceneManager::ChangeEntityModel(uint32_t entityId, const std::string& modelName) {
	currentScene_->ChangeEntityModel(entityId, modelName);
	
}

void SceneManager::ChangeEntityMesh(uint32_t entityId, const std::string& meshName)
{
	currentScene_->ChangeEntityMesh(entityId, meshName);
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

void SceneManager::AddEmptyObject() {
	currentScene_->AddEmptyObject();
}

void SceneManager::AddParticleEmitter(const std::string& modelName, uint32_t maxCount) {
	currentScene_->AddParticleEmitter(modelName, maxCount);
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

void SceneManager::RunTimeAddLuaScript(uint32_t entityId, const std::string& scriptName)
{
	currentScene_->RunTimeAddLuaScript(entityId, scriptName);
}

void SceneManager::StartScript() {
	currentScene_->RunScene();
}

void SceneManager::StopScript() {
	currentScene_->StopScene();
}


