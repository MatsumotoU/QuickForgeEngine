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

std::vector<Camera>& CameraManager::GetCameras() {
	if (cameras_.empty()) {
		assert(!cameras_.empty() && "No cameras available.");
	}

#ifdef _DEBUG
	// 0番以外のカメラだけを返す
	if (cameras_.size() > 1) {
		// 0番以外のカメラだけを格納するstaticなvectorを用意
		static std::vector<Camera> nonDebugCameras;
		nonDebugCameras.clear();
		for (size_t i = 1; i < cameras_.size(); ++i) {
			nonDebugCameras.push_back(cameras_[i]);
		}
		return nonDebugCameras;
	}
	// 0番しかない場合は空のvectorを返す
	static std::vector<Camera> emptyCameras;
	emptyCameras.clear();
	return emptyCameras;
#else
	return cameras_;
#endif // _DEBUG
}