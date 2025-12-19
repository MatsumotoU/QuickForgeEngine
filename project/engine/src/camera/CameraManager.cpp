#include "engine/include/camera/CameraManager.h"
#include <cassert>
#include "engine/include/utility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>
#include "engine/include/assets/AssetManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/scene/Data/SceneObjectData.h"
#include "engine/include/camera/Data/CameraData.h"

void CameraManager::Initialize() {
	nextCameraHandle_ = 0;
	cameras_.clear();
#ifdef _DEBUG
	isActiveDebugCamera_ = false;
#endif // _DEBUG
	
	AddCamera();
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
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
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
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

	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	entityManager->GetComponent<CameraData>(cameras_.at(handle).GetBindEntityId()).handle_ = handle;
	return handle;
}

Camera& CameraManager::GetCamera(uint32_t index) {
	assert(index < cameras_.size() && "Camera index is out of range.");
	return cameras_[index];
}

Camera& CameraManager::GetMainCamera() {
#ifdef _DEBUG
	if (isActiveDebugCamera_) {
		return cameras_[0];
	}
#endif // _DEBUG
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
#ifdef _DEBUG
	if (isActiveDebugCamera_ && cameras_.size() > 1) {
		EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
		Transform& debugCamTransform = entityManager->GetComponent<Transform>(cameras_[0].GetBindEntityId());
		Transform& targetCamTransform = entityManager->GetComponent<Transform>(cameras_[index].GetBindEntityId());
		targetCamTransform = debugCamTransform;

		CameraData& debugCamData = entityManager->GetComponent<CameraData>(cameras_[0].GetBindEntityId());
		CameraData& targetCamData = entityManager->GetComponent<CameraData>(cameras_[index].GetBindEntityId());
		targetCamData = debugCamData;
	}
#endif // _DEBUG
}

//std::vector<Camera>& CameraManager::GetCameras() {
//	if (cameras_.empty()) {
//		assert(!cameras_.empty() && "No cameras available.");
//	}
//
//#ifdef _DEBUG
//	// 0逡ｪ莉･螟悶・繧ｫ繝｡繝ｩ縺縺代ｒ霑斐☆
//	if (cameras_.size() > 1) {
//		// 0逡ｪ莉･螟悶・繧ｫ繝｡繝ｩ縺縺代ｒ譬ｼ邏阪☆繧虐tatic縺ｪvector繧堤畑諢・
//		static std::vector<Camera> nonDebugCameras;
//		nonDebugCameras.clear();
//		for (size_t i = 1; i < cameras_.size(); ++i) {
//			nonDebugCameras.push_back(cameras_[i]);
//		}
//		return nonDebugCameras;
//	}
//	// 0逡ｪ縺励°縺ｪ縺・ｴ蜷医・遨ｺ縺ｮvector繧定ｿ斐☆
//	static std::vector<Camera> emptyCameras;
//	emptyCameras.clear();
//	return emptyCameras;
//#else
//	return cameras_;
//#endif // _DEBUG
//}

void CameraManager::SetMainCameraIndex(uint32_t index) {
	assert(index < cameras_.size() && "Camera index is out of range.");
	mainCameraIndex_ = index;
}
