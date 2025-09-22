#include "CameraManager.h"
#include <cassert>
#include "AppUtility/DebugTool/ImGui/ImGuiInclude.h"
#include <string>

void CameraManager::Initialize() {
	cameras_.clear();
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
	assert(mainCameraIndex_ < cameras_.size() && "MainCameraIndex is out of range.");
	return cameras_[mainCameraIndex_];
}

void CameraManager::DrawImGui() {
	ImGui::Begin("CameraManager");
	for (uint32_t i = 0; i < cameras_.size(); i++) {
		if (ImGui::TreeNode((std::string("Camera") + std::to_string(i)).c_str())) {
			ImGui::DragFloat3("Position", &cameras_[i].transform_.translate.x, 0.1f);
			ImGui::DragFloat3("Rotation", &cameras_[i].transform_.rotate.x, 0.1f);
			ImGui::DragFloat3("Scale", &cameras_[i].transform_.scale.x, 0.1f);
			if (i != mainCameraIndex_) {
				if (ImGui::Button("Set as MainCamera")) {
					mainCameraIndex_ = i;
				}
			} else {
				ImGui::Text("This is MainCamera");
			}
			ImGui::TreePop();
		}
	}
	ImGui::End();
}
