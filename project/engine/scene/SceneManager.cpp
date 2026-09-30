#include "SceneManager.h"

#include "CameraComponent.h"
#include "ObjectInfoComponent.h"
#include "TransformComponent.h"

void QFE::SCENE::SceneManager::Initialize() {
	currentScene_.Initialize();
	++sceneRevision_;
	currentScenePath_.clear();
	CreateCameraEntity();
}

uint32_t QFE::SCENE::SceneManager::CreateCameraEntity(const std::string& name) {
	QFE::EntityManager& entityManager = currentScene_.GetEntityManager();
	bool hasMainCamera = false;
	entityManager.Each<QFE::SCENE::CameraComponent>(
		[&](uint32_t, QFE::SCENE::CameraComponent& camera) {
			hasMainCamera = hasMainCamera || camera.isMainCamera;
		});

	const uint32_t entityId = entityManager.CreateEntity();
	QFE::SCENE::ObjectInfoComponent objectInfo;
	objectInfo.name = name;
	QFE::SCENE::TransformComponent transform;
	transform.transform.translate = { 0.0f, 0.0f, -5.0f };
	QFE::SCENE::CameraComponent camera;
	camera.isMainCamera = !hasMainCamera;

	entityManager.EmplaceComponent<QFE::SCENE::ObjectInfoComponent>(entityId, objectInfo);
	entityManager.EmplaceComponent<QFE::SCENE::TransformComponent>(entityId, transform);
	entityManager.EmplaceComponent<QFE::SCENE::CameraComponent>(entityId, camera);
	return entityId;
}

void QFE::SCENE::SceneManager::EndFrame() {
	currentScene_.EndFrame();
}

void QFE::SCENE::SceneManager::Shutdown() {
	currentScene_.Initialize();
	++sceneRevision_;
}

void QFE::SCENE::SceneManager::SaveCurrentSceneToJson(const std::string& filePath) {
	currentScene_.SaveSceneToJson(filePath);
	currentScenePath_ = filePath;
}

void QFE::SCENE::SceneManager::LoadCurrentSceneFromJson(const std::string& filePath) {
	currentScene_.LoadSceneFromJson(filePath);
	++sceneRevision_;
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
