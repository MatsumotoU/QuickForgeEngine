#include "Player.h"
#include <algorithm>
#include "../../../../Engine/Math/MyMath.h"

void Player::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	isActive_ = true;
}

void Player::Update() {
	if (!isActive_) {
		return;
	}
}

void Player::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}
	model_.Draw(transform_,camera);
}

bool Player::GetIsActive() {
	return isActive_;
}

void Player::SetIsActive(bool isActive) {
	isActive_ = isActive;
}