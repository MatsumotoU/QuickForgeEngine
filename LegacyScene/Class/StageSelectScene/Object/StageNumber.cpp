#include "StageNumber.h"
#include "../Engine/Model/Model.h"
#include "../LegacyScene/StageSelectScene.h"
#include <numbers>

void StageNumber::Initialize(Model *model, const Matrix4x4 &parentWorldMatrix) {
	// モデルの設定
	assert(model);
	model_ = model;

	// 親のワールド行列を設定
	parentWorldMatrix_ = parentWorldMatrix;

	// ワールド変換データを設定
	model_->transform_.translate.y = kHeightOffset;
	model_->transform_.rotate.x = std::numbers::pi_v<float> * 9.0f / 16.0f;

	// モデルの更新
	model_->Update();

	// 親のワールド行列を反映
	model_->worldMatrix_ = model_->worldMatrix_ * parentWorldMatrix_;
}

void StageNumber::Update() {
	// アニメーションの更新
	model_->transform_.translate.y = std::sin(theta_) / 16.0f + kHeightOffset;
	theta_ += std::numbers::pi_v<float> *2.0f / (60.0f * kRotateTime);
	theta_ = std::fmodf(theta_, 2.0f * std::numbers::pi_v<float>);

	// モデルの更新
	model_->Update();

	// 親のワールド行列を反映
	model_->worldMatrix_ = model_->worldMatrix_ * parentWorldMatrix_;
}

void StageNumber::Draw() {
	model_->Draw();
}

void StageNumber::Reset() {
	// アニメーションのリセット
	model_->transform_.translate.y = 1.0f;
	theta_ = 0.0f;

	// モデルの更新
	model_->Update();

	// 親のワールド行列を反映
	model_->worldMatrix_ = model_->worldMatrix_ * parentWorldMatrix_;
}