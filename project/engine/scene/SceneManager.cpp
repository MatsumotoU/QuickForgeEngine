#include "SceneManager.h"

void QFE::SCENE::SceneManager::Initialize() {
	currentScene_.Initialize();
	currentScenePath_.clear();
}

void QFE::SCENE::SceneManager::EndFrame() {
	currentScene_.EndFrame();
}

void QFE::SCENE::SceneManager::Shutdown() {
	currentScene_.Initialize();
}

void QFE::SCENE::SceneManager::SaveCurrentSceneToJson(const std::string& filePath) {
	currentScene_.SaveSceneToJson(filePath);
	currentScenePath_ = filePath;
}

void QFE::SCENE::SceneManager::LoadCurrentSceneFromJson(const std::string& filePath) {
	currentScene_.LoadSceneFromJson(filePath);
	currentScenePath_ = filePath;
}

const std::string& QFE::SCENE::SceneManager::GetCurrentScenePath() const {
	return currentScenePath_;
}

nlohmann::json QFE::SCENE::SceneManager::LoadCurrentSceneToJson(const std::string& filePath) {
	SceneObject scene;
	scene.LoadSceneFromJson(filePath);
	return scene.GetEntityManager().Serialize();
}

uint32_t QFE::SCENE::SceneManager::LoadEntityOnCurrentSceneFromJsonObject(const std::string& filePath) {
	return currentScene_.LoadEntityFromJsonObject(filePath);
}

QFE::EntityManager& QFE::SCENE::SceneManager::GetCurrentSceneEntityManager() {
	return currentScene_.GetEntityManager();
}
