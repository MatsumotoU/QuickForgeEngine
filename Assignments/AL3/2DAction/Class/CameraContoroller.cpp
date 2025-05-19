#include "CameraContoroller.h"
#include "../../../../Engine/Math/MyMath.h"

void CameraContoroller::Initialize(Camera* camera) {
	camera_ = camera;
	targetPosition_ = nullptr;
	targetVelocity_ = nullptr;

	kLimitMinX = 13.0f;
	kLimitMaxX = 187.0f;
}

void CameraContoroller::Update() {
	if (targetPosition_ != nullptr) {
		if (targetVelocity_ != nullptr) {
			MyMath::SimpleEaseIn(&cameraPosition_.x, targetPosition_->x + targetVelocity_->x, 0.1f);
			MyMath::SimpleEaseIn(&cameraPosition_.y, targetPosition_->y , 0.1f);
		} else {
			MyMath::SimpleEaseIn(&cameraPosition_.x, targetPosition_->x, 0.1f);
			MyMath::SimpleEaseIn(&cameraPosition_.y, targetPosition_->y, 0.1f);
		}
	}

	cameraPosition_.z = camera_->transform_.translate.z;
	camera_->transform_.translate = cameraPosition_;
	camera_->transform_.translate.y = std::clamp(camera_->transform_.translate.y, kLimitMinY, kLimitMaxY);
	camera_->transform_.translate.x = std::clamp(camera_->transform_.translate.x, kLimitMinX, kLimitMaxX);
}

void CameraContoroller::SetTargetVelocity(Vector3* targetVelocity) {
	targetVelocity_ = targetVelocity;
}

void CameraContoroller::SetTargetPosition(Vector3* targetPosition) {
	targetPosition_ = targetPosition;
}
