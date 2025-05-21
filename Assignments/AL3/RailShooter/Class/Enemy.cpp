#include "Enemy.h"

void Enemy::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Triangle.obj", COORDINATESYSTEM_HAND_RIGHT);
	isActive_ = false;

	velocity_ = { 0.0f,0.0f,0.0f };

	transform_.rotate.x = 3.14f * 0.5f;
	transform_.rotate.y = 3.14f;
}

void Enemy::Update() {
	if (!isActive_) {
		return;
	}

	transform_.translate += velocity_ * engineCore_->GetDeltaTime();
	if (transform_.translate.z <= -kZLimit) {
		isActive_ = false;
	}
}

void Enemy::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}

	model_.Draw(transform_, camera);
}

void Enemy::Spawn(Vector3 position, Vector3 velocity) {
	if (!isActive_) {
		isActive_ = true;
		transform_.translate = position;
		velocity_ = velocity;
	}
}

bool Enemy::GetIsActive() {
	return isActive_;
}
