#include "engine/include/camera/CameraManager.h"
#include <cassert>
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/camera/Data/CameraData.h"

using namespace QFE;

void CameraManager::Initialize() {
	nextCameraHandle_ = 0;
	cameras_.clear();
#ifdef QFE_OPTIMIZE_OFF
	isActiveDebugCamera_ = false;
#endif // QFE_OPTIMIZE_OFF

	AddCamera();
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	entityManager->GetComponent<SceneObjectData>(cameras_[0].GetBindEntityId()).name = "DebugCamera";

	// 繝・ヵ繧ｩ繝ｫ繝医き繝｡繝ｩ繧定ｿｽ蜉
	mainCameraIndex_ = AddCamera();
}

void CameraManager::Shutdown() {
	cameras_.clear();
}

void CameraManager::Update() {
	for (auto& [id, camera] : cameras_) {
		camera.Update();
	}
}

void CameraManager::Reset() {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	for (auto& [id, camera] : cameras_) {
		entityManager->InstantRemoveEntity(camera.GetBindEntityId());
	}
	cameras_.clear();
	nextCameraHandle_ = 0;
}

uint32_t CameraManager::AddCamera() {
	uint32_t handle = nextCameraHandle_++;
	cameras_.emplace(handle, Camera{});
	cameras_.at(handle).Initialize();

	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	entityManager->GetComponent<CameraData>(cameras_.at(handle).GetBindEntityId()).handle_ = handle;
	return handle;
}

Camera& CameraManager::GetCamera(uint32_t index) {
	assert(index < cameras_.size() && "Camera index is out of range.");
	return cameras_[index];
}

Camera& CameraManager::GetMainCamera() {
#ifdef QFE_OPTIMIZE_OFF
	if (isActiveDebugCamera_) {
		return cameras_[0];
	}
#endif // QFE_OPTIMIZE_OFF
	assert(!cameras_.empty() && "No cameras available.");
	assert(mainCameraIndex_ < cameras_.size() && "MainCameraIndex is out of range.");
	return cameras_[mainCameraIndex_];
}

std::unordered_map<uint32_t, Camera>& CameraManager::GetAllCameras() {
	return cameras_;
}

void CameraManager::SnapToDebugCamera(uint32_t index) {
	assert(index < cameras_.size() && "Camera index is out of range.");
	index;
#ifdef QFE_OPTIMIZE_OFF
	if (isActiveDebugCamera_ && cameras_.size() > 1) {
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
		Transform& debugCamTransform = entityManager->GetComponent<Transform>(cameras_[0].GetBindEntityId());
		Transform& targetCamTransform = entityManager->GetComponent<Transform>(cameras_[index].GetBindEntityId());
		targetCamTransform = debugCamTransform;

		CameraData& debugCamData = entityManager->GetComponent<CameraData>(cameras_[0].GetBindEntityId());
		CameraData& targetCamData = entityManager->GetComponent<CameraData>(cameras_[index].GetBindEntityId());
		targetCamData = debugCamData;
	}
#endif // QFE_OPTIMIZE_OFF
}

void CameraManager::SetMainCameraIndex(uint32_t index) {
	assert(index < cameras_.size() && "Camera index is out of range.");
	mainCameraIndex_ = index;
}
