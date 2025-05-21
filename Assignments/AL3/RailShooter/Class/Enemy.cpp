#include "Enemy.h"

void Enemy::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Triangle.obj", COORDINATESYSTEM_HAND_RIGHT);
	isActive_ = false;

	velocity_ = { 0.0f,0.0f,0.0f };

	transform_.rotate.x = 3.14f * 0.5f;
	transform_.rotate.y = 3.14f;
	leaveSpeed_ = 3.0f;
	phase_ = Phase::Approach;
}

void Enemy::Update() {
	if (!isActive_) {
		return;
	}

	switch (phase_)
	{
	case Phase::Approach:
		transform_.translate += velocity_ * engineCore_->GetDeltaTime();
		if (transform_.translate.z <= 0.0f) {
			phase_ = Phase::Leave;
		}
		break;

	case Phase::Leave:
		if (transform_.translate.Normalize().x == 0.0f) {
			transform_.translate.x += leaveSpeed_ * engineCore_->GetDeltaTime();
		} else {
			transform_.translate.x += transform_.translate.Normalize().x * engineCore_->GetDeltaTime()* leaveSpeed_;
		}
		
		break;
	default:
		break;
	}

	
	if (fabsf(transform_.translate.x) >= kXLimit) {
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
