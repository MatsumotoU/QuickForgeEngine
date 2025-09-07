#include "StageObject.h"
#include "../Engine/Model/Model.h"
#include "../LegacyScene/StageSelectScene.h"
#include <numbers>

void StageObject::Initialize(Model *model, uint32_t stageNumber) {
	// モデルの設定
	assert(model);
	model_ = model;

	// 座標
	Vector3 position = { 5.0f, 0.0f, 0.0f };

	// 回転角
	float angle = static_cast<float>(StageSelectScene::kNumStage - 1 - stageNumber) * 2.0f * std::numbers::pi_v<float> / StageSelectScene::kNumStage;

	// Y軸周りの回転行列
	Matrix4x4 rotateYMatrix = Matrix4x4::MakeRotateYMatrix(angle);

	// 座標を設定
	model_->transform_.translate = Vector3::Transform(position, rotateYMatrix);

	// 姿勢を設定
	model_->transform_.rotate.x = std::numbers::pi_v<float> / 2.0f;
	model_->transform_.rotate.y = angle - std::numbers::pi_v<float> / 2.0f;
}

void StageObject::Update() {
	// アニメーションの更新
	model_->transform_.translate.y = std::sin(theta_) / 16.0f;
	theta_ += std::numbers::pi_v<float> / 120.0f;
	theta_ = std::fmodf(theta_, 2.0f * std::numbers::pi_v<float>);

	// モデルの更新
	model_->Update();
}

void StageObject::Draw() {
	model_->Draw();
}

void StageObject::Reset() {
	// アニメーションのリセット
	model_->transform_.translate.y = 0.0f;
	theta_ = 0.0f;

	// モデルの更新
	model_->Update();
}