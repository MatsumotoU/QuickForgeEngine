#include "RailCameraController.h"

void RailCameraController::Initialize(Camera* camera) {
	camera_ = camera;
}

void RailCameraController::Update() {
	if (camera_ == nullptr) {
		return;
	}
}

Camera* RailCameraController::GetCamera() const {
	return camera_;
}
