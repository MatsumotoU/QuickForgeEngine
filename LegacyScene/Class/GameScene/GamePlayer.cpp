#include "GamePlayer.h"

void GamePlayer::Jamp(const Vector2& dir) {
	if (!isJumping_ && !isGrounded_) {
		isJumping_ = true;
		velocity_ = { dir.x * 2.0f,15.0f,dir.y * 2.0f };
	}
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

bool& GamePlayer::GetIsGrounded() {
	return isGrounded_;
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
