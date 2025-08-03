#include "ShieldBar.h"
#include "Utility/MyEasing.h"

void ShieldBar::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Box.obj", COORDINATESYSTEM_HAND_RIGHT);
	backModel_.Initialize(engineCore_);
	backModel_.LoadModel("Resources", "Box.obj", COORDINATESYSTEM_HAND_RIGHT);

	shieldPoint_ = 0.0f;
	maxShieldPoint_ = 100.0f;
	barWidth_ = 1.0f;
	barHeight_ = 0.2f;
	position_ = { 1.3f, -1.0f, 5.0f };
	color_ = { 0.6f, 0.3f, 0.1f, 1.0f };
	frameCount_ = 0;
}

void ShieldBar::Update() {
	frameCount_++;

	if (shieldPoint_ < 0.0f) {
		shieldPoint_ = 0.0f;
	}
	if (shieldPoint_ > maxShieldPoint_) {
		shieldPoint_ = maxShieldPoint_;
	}
	float shieldRatio = shieldPoint_ / maxShieldPoint_;
	model_.transform_.translate = position_;
	model_.transform_.translate.x += (barWidth_ * 0.5f)-(model_.transform_.scale.x * 0.5f * shieldRatio);
	Eas::SimpleEaseIn(&model_.transform_.scale.x, barWidth_ * shieldRatio, 0.1f);
	model_.transform_.scale.y = barHeight_;
	model_.transform_.scale.z = 0.1f;
	model_.material_.materialData_->color = color_;
	if (shieldPoint_ == maxShieldPoint_) {
		model_.material_.materialData_->color = Vector4(1.0f, 1.0f, 0.0f, 1.0f);
	} 
	model_.Update();

	backModel_.transform_.translate = position_;
	backModel_.transform_.scale = { barWidth_, barHeight_ * 0.5f, 0.1f };
	backModel_.material_.materialData_->color = { 0.7f, 0.7f, 0.7f, 1.0f };
	backModel_.Update();
}

void ShieldBar::Draw(Camera* camera) {
	backModel_.worldMatrix_ = Matrix4x4::Multiply(backModel_.worldMatrix_, camera->GetWorldMatrix());
	backModel_.Draw(camera);

	if (shieldPoint_ <= 0.0f) {
		return;
	}

	model_.worldMatrix_ = Matrix4x4::Multiply(model_.worldMatrix_, camera->GetWorldMatrix());
	model_.Draw(camera);
}
