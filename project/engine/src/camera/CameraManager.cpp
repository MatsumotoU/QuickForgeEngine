#include "engine/include/camera/CameraManager.h"
#include <cassert>
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>
#include "engine/include/assets/AssetManager.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/camera/Data/CameraData.h"
#include "engine/include/core/Math/TransformComponent.h"

using namespace QFE;

void CameraManager::Initialize() {
	nextCameraHandle_ = 0;
	cameras_.clear();
	isActiveDebugCamera_ = false;

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
	if (isActiveDebugCamera_) {
		return cameras_[0];
	}
	assert(!cameras_.empty() && "No cameras available.");
	assert(mainCameraIndex_ < cameras_.size() && "MainCameraIndex is out of range.");
	return cameras_[mainCameraIndex_];
}

const EulerTransform& QFE::CameraManager::GetMainCameraTransform() const {
	EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
	uint32_t mainCameraEntityId = cameras_.at(mainCameraIndex_).GetBindEntityId();
	if (entityManager->HasComponent<TransformComponent>(mainCameraEntityId)) {
		return entityManager->GetComponent<TransformComponent>(mainCameraEntityId).transform;
	} else {
		QFE_REPORT_SYSTEM_ERROR("Main camera entity does not have a EulerTransform component.", SystemError::Abort);
	}
	return dummyCameraTransform_;
}

std::unordered_map<uint32_t, Camera>& CameraManager::GetAllCameras() {
	return cameras_;
}

void CameraManager::SnapToDebugCamera(uint32_t index) {
	assert(index < cameras_.size() && "Camera index is out of range.");
	index;

	if (isActiveDebugCamera_ && cameras_.size() > 1) {
		EntityManager* entityManager = SceneManager::GetInstance()->GetEntityManager();
		EulerTransform& debugCamTransform = entityManager->GetComponent<TransformComponent>(cameras_[0].GetBindEntityId()).transform;
		EulerTransform& targetCamTransform = entityManager->GetComponent<TransformComponent>(cameras_[index].GetBindEntityId()).transform;
		targetCamTransform = debugCamTransform;

		CameraData& debugCamData = entityManager->GetComponent<CameraData>(cameras_[0].GetBindEntityId());
		CameraData& targetCamData = entityManager->GetComponent<CameraData>(cameras_[index].GetBindEntityId());
		targetCamData = debugCamData;
	}
}

void CameraManager::SetMainCameraIndex(uint32_t index) {
	assert(index < cameras_.size() && "Camera index is out of range.");
	mainCameraIndex_ = index;
}
