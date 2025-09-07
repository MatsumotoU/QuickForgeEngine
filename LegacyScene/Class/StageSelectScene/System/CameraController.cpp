#include "CameraController.h"
#include "../Engine/Model/Model.h"
#include <numbers>

void CameraController::Initialize(EngineCore *engineCore, Camera *camera, const Vector3 &targetPosition) {
	// エンジンの中核機能を設定
	assert(engineCore);
	engineCore_ = engineCore;

	// カメラを設定
	assert(camera);
	camera_ = camera;

	// 追従対象の座標を設定
	targetPosition_ = targetPosition;

	// 追従対象の座標とオフセットからカメラの座標を計算
	camera_->transform_.translate = targetPosition_ * 2.0f;

	camera_->transform_.rotate = Vector3::LookAt(camera_->transform_.translate, targetPosition_);
	
	// カメラの行列を更新
	camera_->Update();
}

void CameraController::Update() {
	// 補間にかけた時間を更新
	lerpTimer_++;

	Vector3 targetPosition = targetPosition_ * 2.0f;
	Vector3 lookAt = Vector3::LookAt(targetPosition, targetPosition_);

	// 座標補間
	camera_->transform_.translate = Vector3::Slerp(startPosition_, targetPosition, lerpTimer_ / kLerpDuration);

	// 角度補間
	camera_->transform_.rotate = Vector3::LerpAngle(startRotation_, lookAt, lerpTimer_ / kLerpDuration);

	// カメラの行列を更新
	camera_->Update();

#ifdef _DEBUG
	ImGui::Text("lerpTimer: %.2f", lerpTimer_);
#endif // DEBUG
}

void CameraController::Start() {
	startPosition_ = camera_->transform_.translate;
	startRotation_ = camera_->transform_.rotate;
	lerpTimer_ = 0.0f;
}