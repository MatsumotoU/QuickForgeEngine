#include "CameraManager.h"
#include <cassert>
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>
#include "Assets/AssetManager.h"
#include "Core/Entity/EntityManager.h"
#include "Scene/Data/SceneObjectData.h"
#include "Camera/Data/CameraData.h"

void CameraManager::Initialize() {
	nextCameraHandle_ = 0;
	cameras_.clear();
#ifdef _DEBUG
	isActiveDebugCamera_ = false;
#endif // _DEBUG
	
	AddCamera();
	EntityManager* entityManager = AssetManager::GetInstance()->GetEntityManager();
	entityManager->GetComponent<SceneObjectData>(cameras_[0].GetBindEntityId()).name = "DebugCamera";

	// デフォルトカメラを追加
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
//	// 0番以外のカメラだけを返す
//	if (cameras_.size() > 1) {
//		// 0番以外のカメラだけを格納するstaticなvectorを用意
//		static std::vector<Camera> nonDebugCameras;
//		nonDebugCameras.clear();
//		for (size_t i = 1; i < cameras_.size(); ++i) {
//			nonDebugCameras.push_back(cameras_[i]);
//		}
//		return nonDebugCameras;
//	}
//	// 0番しかない場合は空のvectorを返す
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
