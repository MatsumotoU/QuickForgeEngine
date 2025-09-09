#include "CameraController.h"
#include "../Engine/Model/Model.h"

void CameraController::Initialize(Camera *camera, const Vector3 &targetPosition) {
	// カメラを設定
	assert(camera);
	camera_ = camera;

	// 追従対象の座標を設定
	targetPosition_ = targetPosition;

	// 追従対象の座標からカメラの座標と向きを計算
	camera_->transform_.translate = targetPosition_ + Vector3::Normalize(targetPosition_) * kCameraDistance;
	camera_->transform_.translate.y += kCameraHeight;
	camera_->transform_.rotate = Vector3::LookAt(camera_->transform_.translate, targetPosition_);

	// カメラの行列を更新
	camera_->Update();
}

void CameraController::Update() {
	// 補間にかけた時間を更新
	lerpTimer_++;

	// 追従対象の座標から補間終了後のカメラの座標と向きを計算
	Vector3 targetPosition = targetPosition_ + Vector3::Normalize(targetPosition_) * kCameraDistance;
	targetPosition.y += kCameraHeight;
	Vector3 lookAt = Vector3::LookAt(targetPosition, targetPosition_);

	// 座標補間
	camera_->transform_.translate = Vector3::Slerp(startPosition_, targetPosition, lerpTimer_ / kLerpDuration);

	// 角度補間
	camera_->transform_.rotate = Vector3::LerpAngle(startRotation_, lookAt, lerpTimer_ / kLerpDuration);

	// カメラの行列を更新
	camera_->Update();
}

void CameraController::Start() {
	startPosition_ = camera_->transform_.translate;
	startRotation_ = camera_->transform_.rotate;
	lerpTimer_ = 0.0f;
}