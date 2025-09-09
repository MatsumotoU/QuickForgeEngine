#include "StageSelectAnchor.h"
#include "../LegacyScene/StageSelectScene.h"
#include <numbers>

void StageSelectAnchor::Initialize(uint32_t stageNumber) {
	// 座標
	Vector3 position = { 5.0f, 0.0f, 0.0f };

	// 回転角
	float angle = static_cast<float>(StageSelectScene::kNumStage - 1 - stageNumber) * 2.0f * std::numbers::pi_v<float> / StageSelectScene::kNumStage;

	// Y軸周りの回転行列
	Matrix4x4 rotateYMatrix = Matrix4x4::MakeRotateYMatrix(angle);

	// 座標を設定
	transform_.translate = Vector3::Transform(position, rotateYMatrix);
	transform_.rotate.y = angle + std::numbers::pi_v<float> * 1.5f;

	// ワールド行列を設定
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}