#include "SceneManager.h"

#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "ObjectInfoComponent.h"
#include "TransformComponent.h"

QFE::SCENE::SceneManager::SceneManager(
	std::shared_ptr<const QFE::StaticComponentRegistry> registry) :
	registry_(std::move(registry)), currentScene_(registry_) {
}

void QFE::SCENE::SceneManager::Initialize() {
	currentScene_.Initialize();
	++sceneRevision_;
	currentScenePath_.clear();
	CreateCameraEntity();
	CreateDirectionalLightEntity("MainDirectionalLight");
}

uint32_t QFE::SCENE::SceneManager::CreateDirectionalLightEntity(const std::string& name) {
	QFE::EntityManager& entityManager = currentScene_.GetEntityManager();
	bool hasMainLight = false;
	entityManager.Each<QFE::SCENE::DirectionalLightComponent>(
		[&](uint32_t, QFE::SCENE::DirectionalLightComponent& light) {
			hasMainLight = hasMainLight || light.isMainLight;
		});

	const uint32_t entityId = entityManager.CreateEntity();
	QFE::SCENE::ObjectInfoComponent objectInfo;
	objectInfo.name = name;
	QFE::SCENE::DirectionalLightComponent light;
	light.isMainLight = !hasMainLight;
	entityManager.EmplaceComponent<QFE::SCENE::ObjectInfoComponent>(entityId, objectInfo);
	entityManager.EmplaceComponent<QFE::SCENE::TransformComponent>(entityId);
	entityManager.EmplaceComponent<QFE::SCENE::DirectionalLightComponent>(entityId, light);
	return entityId;
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

bool QFE::SCENE::SceneManager::LoadCurrentSceneFromJson(const nlohmann::json& sceneJson) {
	if (!currentScene_.LoadSceneFromJson(sceneJson)) {
		return false;
	}
	++sceneRevision_;
	return true;
}

QFE::SCENE::SceneSnapshot QFE::SCENE::SceneManager::CreateSnapshot() const {
	return SceneSnapshot{
		.sceneJson = currentScene_.SerializeToJson(),
		.scenePath = currentScenePath_,
	};
}

void QFE::SCENE::SceneManager::ReleaseComponentRegistryForReload() {
	currentScene_.ReleaseComponentRegistryForReload();
	registry_.reset();
}

bool QFE::SCENE::SceneManager::RestoreAfterComponentReload(
	std::shared_ptr<const QFE::StaticComponentRegistry> registry,
	const SceneSnapshot& snapshot) {
	if (!registry || !registry->IsFrozen()) {
		return false;
	}

	registry_ = std::move(registry);
	currentScene_.BindComponentRegistryAfterReload(registry_);
	if (!currentScene_.LoadSceneFromJson(snapshot.sceneJson)) {
		currentScene_.ReleaseComponentRegistryForReload();
		registry_.reset();
		return false;
	}

	currentScenePath_ = snapshot.scenePath;
	++sceneRevision_;
	return true;
}

const std::string& QFE::SCENE::SceneManager::GetCurrentScenePath() const {
	return currentScenePath_;
}

nlohmann::json QFE::SCENE::SceneManager::LoadCurrentSceneToJson(const std::string& filePath) {
	SceneObject scene(registry_);
	scene.LoadSceneFromJson(filePath);
	return scene.SerializeToJson();
}

uint32_t QFE::SCENE::SceneManager::LoadEntityOnCurrentSceneFromJsonObject(const std::string& filePath) {
	return currentScene_.LoadEntityFromJsonObject(filePath);
}

QFE::EntityManager& QFE::SCENE::SceneManager::GetCurrentSceneEntityManager() {
	return currentScene_.GetEntityManager();
}
