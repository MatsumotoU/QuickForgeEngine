#include "SceneManager.h"

#include "CameraComponent.h"
#include "DirectionalLightComponent.h"
#include "ObjectInfoComponent.h"
#include "TransformComponent.h"
#include <algorithm>
#include <fstream>

QFE::SCENE::SceneManager::SceneManager(
	std::shared_ptr<const QFE::StaticComponentRegistry> registry) :
	registry_(std::move(registry)), currentScene_(registry_) {
}

void QFE::SCENE::SceneManager::Initialize() {
	subScenes_.clear();
	nextSceneId_ = 1;
	activeSceneId_ = MainSceneId;
	mainUpdating_ = true;
	mainVisible_ = true;
	currentScene_.Initialize();
	sceneRevision_ = ++revisionCounter_;
	currentScenePath_.clear();
	CreateCameraEntity();
	CreateDirectionalLightEntity("MainDirectionalLight");
}

uint32_t QFE::SCENE::SceneManager::CreateDirectionalLightEntity(const std::string& name) {
	QFE::EntityManager& entityManager = GetCurrentSceneEntityManager();
	const uint32_t entityId = entityManager.CreateEntity();
	QFE::SCENE::ObjectInfoComponent objectInfo;
	objectInfo.name = name;
	QFE::SCENE::DirectionalLightComponent light;
	entityManager.EmplaceComponent<QFE::SCENE::ObjectInfoComponent>(entityId, objectInfo);
	entityManager.EmplaceComponent<QFE::SCENE::TransformComponent>(entityId);
	entityManager.EmplaceComponent<QFE::SCENE::DirectionalLightComponent>(entityId, light);
	return entityId;
}

uint32_t QFE::SCENE::SceneManager::CreateCameraEntity(const std::string& name) {
	QFE::EntityManager& entityManager = GetCurrentSceneEntityManager();
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
	for (auto& sub : subScenes_) sub.object->EndFrame();
}

void QFE::SCENE::SceneManager::Shutdown() {
	subScenes_.clear();
	activeSceneId_ = MainSceneId;
	currentScene_.Initialize();
	sceneRevision_ = ++revisionCounter_;
}

void QFE::SCENE::SceneManager::SaveCurrentSceneToJson(const std::string& filePath) {
	if (activeSceneId_ != MainSceneId) {
		for (auto& sub : subScenes_) if (sub.id == activeSceneId_) {
			sub.object->SaveSceneToJson(filePath);
			sub.path = filePath;
			return;
		}
	}
	currentScene_.SaveSceneToJson(filePath);
	currentScenePath_ = filePath;
}

void QFE::SCENE::SceneManager::LoadCurrentSceneFromJson(const std::string& filePath) {
	if (activeSceneId_ != MainSceneId) {
		if (!ReplaceScene(activeSceneId_, filePath)) QFE_LOG("Failed to load scene: " + filePath, QFE::LogLevel::Error);
		return;
	}
	currentScene_.LoadSceneFromJson(filePath);
	sceneRevision_ = ++revisionCounter_;
	currentScenePath_ = filePath;
}

bool QFE::SCENE::SceneManager::LoadCurrentSceneFromJson(const nlohmann::json& sceneJson) {
	if (activeSceneId_ != MainSceneId) {
		for (auto& sub : subScenes_) if (sub.id == activeSceneId_) {
			if (!sub.object->LoadSceneFromJson(sceneJson)) return false;
			sub.revision = ++revisionCounter_;
			return true;
		}
		return false;
	}
	if (!currentScene_.LoadSceneFromJson(sceneJson)) {
		return false;
	}
	sceneRevision_ = ++revisionCounter_;
	return true;
}

QFE::SCENE::SceneSnapshot QFE::SCENE::SceneManager::CreateSnapshot() const {
	SceneSnapshot snapshot{
		.sceneJson = currentScene_.SerializeToJson(),
		.scenePath = currentScenePath_,
		.updating = mainUpdating_,
		.visible = mainVisible_,
		.subScenes = {},
	};
	for (const auto& sub : subScenes_) {
		snapshot.subScenes.push_back({ sub.id, sub.name, sub.path,
			sub.object->SerializeToJson(), sub.updating, sub.visible });
	}
	return snapshot;
}

bool QFE::SCENE::SceneManager::RestoreSnapshot(const SceneSnapshot& snapshot) {
	std::vector<SubScene> restored;
	SceneId nextId = 1;
	for (const auto& saved : snapshot.subScenes) {
		auto object = std::make_unique<SceneObject>(registry_);
		if (!object->LoadSceneFromJson(saved.sceneJson)) return false;
		restored.push_back({ saved.id, saved.name, saved.scenePath,
			std::move(object), saved.updating, saved.visible, ++revisionCounter_ });
		nextId = (std::max)(nextId, saved.id + 1);
	}
	// 退避データの先頭は常にメインシーンであり、処理対象の切り替えに依存させない。
	if (!currentScene_.LoadSceneFromJson(snapshot.sceneJson)) return false;
	sceneRevision_ = ++revisionCounter_;
	currentScenePath_ = snapshot.scenePath;
	mainUpdating_ = snapshot.updating;
	mainVisible_ = snapshot.visible;
	subScenes_ = std::move(restored);
	activeSceneId_ = MainSceneId;
	nextSceneId_ = nextId;
	return true;
}

void QFE::SCENE::SceneManager::ReleaseComponentRegistryForReload() {
	currentScene_.ReleaseComponentRegistryForReload();
	for (auto& sub : subScenes_) sub.object->ReleaseComponentRegistryForReload();
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
	if (!RestoreSnapshot(snapshot)) {
		currentScene_.ReleaseComponentRegistryForReload();
		registry_.reset();
		return false;
	}
	return true;
}

const std::string& QFE::SCENE::SceneManager::GetCurrentScenePath() const {
	const std::string* path = GetScenePath(activeSceneId_);
	return path ? *path : currentScenePath_;
}

nlohmann::json QFE::SCENE::SceneManager::LoadCurrentSceneToJson(const std::string& filePath) {
	SceneObject scene(registry_);
	scene.LoadSceneFromJson(filePath);
	return scene.SerializeToJson();
}

uint32_t QFE::SCENE::SceneManager::LoadEntityOnCurrentSceneFromJsonObject(const std::string& filePath) {
	for (auto& sub : subScenes_) if (sub.id == activeSceneId_) return sub.object->LoadEntityFromJsonObject(filePath);
	return currentScene_.LoadEntityFromJsonObject(filePath);
}

QFE::EntityManager& QFE::SCENE::SceneManager::GetCurrentSceneEntityManager() {
	return *GetSceneEntityManager(activeSceneId_);
}

uint32_t QFE::SCENE::SceneManager::AppendScene(const std::string& filePath, const QFE::MATH::Vector3& origin) {
	for (auto& sub : subScenes_) if (sub.id == activeSceneId_) return sub.object->AppendSceneFromJson(filePath, origin);
	return currentScene_.AppendSceneFromJson(filePath, origin);
}

QFE::SCENE::SceneId QFE::SCENE::SceneManager::LoadSubScene(const std::string& name, const std::string& filePath) {
	if (name.empty() || filePath.empty()) return InvalidSceneId;
	const SceneId existing = FindScene(name);
	if (existing != InvalidSceneId) return existing;
	const std::filesystem::path path(std::u8string(
		reinterpret_cast<const char8_t*>(filePath.data()), filePath.size()));
	std::ifstream file(path);
	if (!file) return InvalidSceneId;
	const auto json = nlohmann::json::parse(file, nullptr, false);
	if (json.is_discarded()) return InvalidSceneId;
	auto object = std::make_unique<SceneObject>(registry_);
	if (!object->LoadSceneFromJson(json)) return InvalidSceneId;
	const SceneId id = nextSceneId_++;
	// 失敗時に管理対象を増やさないよう、読み込み後に登録する。
	subScenes_.push_back({ id, name, filePath, std::move(object), true, true, ++revisionCounter_ });
	return id;
}

bool QFE::SCENE::SceneManager::ReplaceScene(SceneId id, const std::string& filePath) {
	const std::filesystem::path path(std::u8string(
		reinterpret_cast<const char8_t*>(filePath.data()), filePath.size()));
	std::ifstream file(path);
	if (!file) return false;
	const auto json = nlohmann::json::parse(file, nullptr, false);
	if (json.is_discarded()) return false;
	if (id == MainSceneId) {
		if (!currentScene_.LoadSceneFromJson(json)) return false;
		sceneRevision_ = ++revisionCounter_;
		currentScenePath_ = filePath;
		return true;
	}
	for (auto& sub : subScenes_) {
		if (sub.id != id) continue;
		// 先に別の SceneObject へ読み込み、失敗時に元のシーンを残す。
		auto object = std::make_unique<SceneObject>(registry_);
		if (!object->LoadSceneFromJson(json)) return false;
		sub.object = std::move(object);
		sub.path = filePath;
		sub.revision = ++revisionCounter_;
		return true;
	}
	return false;
}

QFE::SCENE::SceneId QFE::SCENE::SceneManager::FindScene(const std::string& name) const {
	if (name == "Main") return MainSceneId;
	for (const auto& sub : subScenes_) if (sub.name == name) return sub.id;
	return InvalidSceneId;
}

QFE::EntityManager* QFE::SCENE::SceneManager::GetSceneEntityManager(SceneId id) {
	if (id == MainSceneId) return &currentScene_.GetEntityManager();
	for (auto& sub : subScenes_) if (sub.id == id) return &sub.object->GetEntityManager();
	return nullptr;
}

const std::string* QFE::SCENE::SceneManager::GetScenePath(SceneId id) const {
	if (id == MainSceneId) return &currentScenePath_;
	for (const auto& sub : subScenes_) if (sub.id == id) return &sub.path;
	return nullptr;
}

uint64_t QFE::SCENE::SceneManager::GetSceneRevision(SceneId id) const {
	if (id == MainSceneId) return sceneRevision_;
	for (const auto& sub : subScenes_) if (sub.id == id) return sub.revision;
	return 0;
}

bool QFE::SCENE::SceneManager::SetSceneUpdating(SceneId id, bool updating) {
	if (id == MainSceneId) { mainUpdating_ = updating; return true; }
	for (auto& sub : subScenes_) if (sub.id == id) { sub.updating = updating; return true; }
	return false;
}

bool QFE::SCENE::SceneManager::SetSceneVisible(SceneId id, bool visible) {
	if (id == MainSceneId) { mainVisible_ = visible; return true; }
	for (auto& sub : subScenes_) if (sub.id == id) { sub.visible = visible; return true; }
	return false;
}

bool QFE::SCENE::SceneManager::IsSceneUpdating(SceneId id) const {
	if (id == MainSceneId) return mainUpdating_;
	for (const auto& sub : subScenes_) if (sub.id == id) return sub.updating;
	return false;
}

bool QFE::SCENE::SceneManager::IsSceneVisible(SceneId id) const {
	if (id == MainSceneId) return mainVisible_;
	for (const auto& sub : subScenes_) if (sub.id == id) return sub.visible;
	return false;
}

std::vector<QFE::SCENE::SceneId> QFE::SCENE::SceneManager::GetSceneIds() const {
	std::vector<SceneId> ids{ MainSceneId };
	for (const auto& sub : subScenes_) ids.push_back(sub.id);
	return ids;
}

bool QFE::SCENE::SceneManager::SetCurrentScene(SceneId id) {
	if (!GetSceneEntityManager(id)) return false;
	activeSceneId_ = id;
	return true;
}
