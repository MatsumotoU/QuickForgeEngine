#include "SceneManager.h"

void QFE::SCENE::SceneManager::Initialize() {
	currentScene_.Initialize();
}

void QFE::SCENE::SceneManager::EndFrame() {
	currentScene_.EndFrame();
}

void QFE::SCENE::SceneManager::Shutdown() {
	currentScene_.Initialize();
}

void QFE::SCENE::SceneManager::SaveCurrentSceneToJson(const std::string& filePath) {
	currentScene_.SaveSceneToJson(filePath);
}

void QFE::SCENE::SceneManager::LoadCurrentSceneFromJson(const std::string& filePath) {
	currentScene_.LoadSceneFromJson(filePath);
}

nlohmann::json QFE::SCENE::SceneManager::LoadCurrentSceneToJson(const std::string& filePath) {
	SceneObject scene;
	scene.LoadSceneFromJson(filePath);
	return scene.GetEntityManager().Serialize();
}

QFE::EntityManager& QFE::SCENE::SceneManager::GetCurrentSceneEntityManager() {
	return currentScene_.GetEntityManager();
}
