#include "GamePlayer.h"

void GamePlayer::ResetForce() {
	velocity_ = { 0.0f,0.0f,0.0f };
	acceleration_ = { 0.0f,0.0f,0.0f };
	isJumping_ = false;
}

void GamePlayer::RestParameter() {
	accelerationDamping_ = 0.95f;
	velocityDamping_ = 0.98f;

	isAlive_ = true;
	isCanMove_ = false;
	isCanShot_ = false;
	isMoving_ = false;
	isReqestBuilding_ = false;
	isGrounded_ = false;

	moveTimer_ = 0.0f;
	maxMoveTimer_ = 1.0f;
	shotPower_ = 6.0f;
	moveDir_ = { 0.0f,0.0f };

	transform_.rotate = { 0.0f,0.0f,0.0f };
	transform_.scale = {1.0f,1.0f,1.0f};
	transform_.translate = { 0.0f,0.0f,0.0f };
	model_->transform_ = transform_;
	model_->Update();
}

void GamePlayer::Jamp(const Vector2& dir) {
	if (!isJumping_ && !isGrounded_) {
		isJumping_ = true;
		velocity_ = { dir.x * 2.5f,15.0f,dir.y * 2.5f };
	}
}

void GamePlayer::DeathAnimation() {
	if (model_->transform_.scale.x <= 0.1f) {
		return;
	}

	float deltaTime = engineCore_->GetDeltaTime();
	MyEasing::SimpleEaseIn(&model_->transform_.scale.x, 0.0f, 0.01f);
	MyEasing::SimpleEaseIn(&model_->transform_.scale.y, 0.0f, 0.01f);
	MyEasing::SimpleEaseIn(&model_->transform_.scale.z, 0.0f, 0.01f);
	model_->transform_.rotate.y += 10.0f * deltaTime;

	model_->transform_.translate += velocity_ * deltaTime;
	velocity_ = velocity_ * velocityDamping_;

	if (velocity_.y > -5.0f) {
		velocity_.y -= 9.81f * deltaTime;
	}

	model_->Update();
}

Vector4 GamePlayer::GetColor() const {
	return color_;
}

Vector2& GamePlayer::GetMoveDir() {
	return moveDir_;
}

Vector3 GamePlayer::GetWorldPosition() {
	return Vector3::Transform({ 0.0f,0.0f,0.0f }, model_->worldMatrix_);
}

Transform& GamePlayer::GetTransform() {
	return model_->transform_;
}

bool& GamePlayer::GetIsCanMove() {
	return isCanMove_;
}

bool& GamePlayer::GetIsMoving() {
	return isMoving_;
}

bool& GamePlayer::GetIsBuilding() {
	return isReqestBuilding_;
}

bool& GamePlayer::GetIsJumping() {
	return isJumping_;
}

bool& GamePlayer::GetIsGrounded() {
	return isGrounded_;
}

bool GamePlayer::GetIsEndTurn() {
	// 縺吶∋縺ｦ縺ｮ迥ｶ諷九′false縺ｮ縺ｨ縺阪↓繧ｿ繝ｼ繝ｳ邨ゆｺ・
	return !isMoving_ && !isCanMove_ && !isReqestBuilding_ && !isJumping_ && !isGrounded_;
}

bool GamePlayer::GetIsAlive() const {
	return isAlive_;
}

float GamePlayer::GetMoveTimer() const {
	return maxMoveTimer_ * shotPower_;
}

void GamePlayer::SetMoveDir(const Vector2& dir) {
	moveDir_ = dir;
}

void GamePlayer::SetIsBuilding(bool set) {
	isReqestBuilding_ = set;
}

void GamePlayer::SetGrounded(bool set) {
	isGrounded_ = set;
}

void GamePlayer::SetAlpha(float alpha) {
	alpha_ = alpha;
}

void GamePlayer::SetAlive(bool set) {
	isAlive_ = set;
}

void GamePlayer::SetMap(std::vector<std::vector<uint32_t>>* floor, std::vector<std::vector<uint32_t>>* wall) {
	floorMap_ = floor;
	wallMap_ = wall;
}
