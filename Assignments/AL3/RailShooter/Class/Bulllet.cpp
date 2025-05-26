#include "Bulllet.h"

void Bullet::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Triangle.obj", COORDINATESYSTEM_HAND_RIGHT);
	isActive_ = false;

	transform_.rotate.x = 3.14f * 0.5f;
	transform_.scale.y = 2.0f;
}

void Bullet::Update() {
	if (isActive_) {
		transform_.translate += velocity_ * engineCore_->GetDeltaTime();

		if (aliveTime_ > 0) {
			aliveTime_--;
		} else {
			isActive_ = false;
		}
	}
}

void Bullet::Draw(Camera* camera) {
	if (isActive_) {
		model_.Draw(transform_, camera);
	}
}

void Bullet::ShotBullet(Vector3 position, Vector3 velocity, uint32_t aliveTime) {
	transform_.translate = position;
	velocity_ = velocity;
	aliveTime_ = aliveTime;
	maxAliveTime_ = aliveTime;
	isActive_ = true;
}

bool Bullet::GetIsActive() {
	return isActive_;
}
