#include "CameraManager.h"
#include <cassert>
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>

void CameraManager::Initialize() {
	cameras_.clear();
#ifdef _DEBUG
	isActiveDebugCamera_ = false;
	AddCamera();
#endif // _DEBUG
	// デフォルトカメラを追加
	mainCameraIndex_ = AddCamera();
}

void CameraManager::Shutdown() {
	cameras_.clear();
}

void CameraManager::Update() {
	for (auto& camera : cameras_) {
		camera.Update();
	}
}

uint32_t CameraManager::AddCamera() {
	cameras_.emplace_back();
	cameras_.back().Initialize();
	return static_cast<uint32_t>(cameras_.size() - 1);
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

	assert(mainCameraIndex_ < cameras_.size() && "MainCameraIndex is out of range.");
	return cameras_[mainCameraIndex_];
}