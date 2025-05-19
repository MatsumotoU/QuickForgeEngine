#include "Player.h"
#include <algorithm>
#include "../../../../Engine/Math/MyMath.h"

void Player::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	moveSpeed_ = 50.0f;

	isActive_ = true;
}

void Player::Update() {
	if (!isActive_) {
		return;
	}

	DirectInputManager* input = engineCore_->GetInputManager();
	
	acceleration_.x += input->GetKeyMoveDir().x * moveSpeed_ * engineCore_->GetDeltaTime();
	acceleration_.y += input->GetKeyMoveDir().y * moveSpeed_ * engineCore_->GetDeltaTime();

	acceleration_.x = std::clamp(acceleration_.x, -kLimitSpeed, kLimitSpeed);
	acceleration_.y = std::clamp(acceleration_.y, -kLimitSpeed, kLimitSpeed);

	velocity_ += acceleration_ * engineCore_->GetDeltaTime();
	acceleration_ = acceleration_ * 0.95f;

	velocity_.x = std::clamp(velocity_.x, -kLimitSpeed, kLimitSpeed);
	velocity_.y = std::clamp(velocity_.y, -kLimitSpeed, kLimitSpeed);

	transform_.translate += velocity_ * engineCore_->GetDeltaTime();
	velocity_ = velocity_ * 0.98f;

	transform_.translate.x = std::clamp(transform_.translate.x, -kLimitMoveWidh, kLimitMoveWidh);
	transform_.translate.y = std::clamp(transform_.translate.y, -kLimitMoveHeight, kLimitMoveHeight);

	if (input->keyboard_.GetTrigger(DIK_SPACE)) {
		if (!isShot_) {
			isShot_ = true;
		}
	}
}

void Player::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}
	model_.Draw(transform_,camera);

	ImGui::Begin("Player");
	ImGui::DragFloat3("translate", &transform_.translate.x);
	ImGui::DragFloat3("velocity", &velocity_.x);
	ImGui::DragFloat3("acceleration", &acceleration_.x);
	ImGui::End();
}

bool Player::GetIsActive() {
	return isActive_;
}

bool Player::GetIsShot() {
	return isShot_;
}

void Player::SetIsActive(bool isActive) {
	isActive_ = isActive;
}

void Player::SetIsShot(bool isShot) {
	isShot_ = isShot;
}
