#include "Bulllet.h"

Bullet::Bullet() {
	color_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Bullet::Initialize(EngineCore* engineCore, const std::string& name) {
	name_ = name;
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	if (name_ == "Player") {
		model_.LoadModel("Resources", "Bullet.obj", COORDINATESYSTEM_HAND_RIGHT);
	} else {
		model_.LoadModel("Resources", "missile.obj", COORDINATESYSTEM_HAND_RIGHT);
	}
	
	isActive_ = false;

	transform_.scale = Vector3(0.5f, 0.5f, 0.5f);
	

}

void Bullet::Update() {
	if (isActive_) {
		transform_.translate += velocity_ * engineCore_->GetDeltaTime();
		transform_.rotate = -Vector3::LookAt(transform_.translate, transform_.translate + velocity_);


		if (aliveTime_ > 0) {
			aliveTime_--;
		} else {
			isActive_ = false;
		}
	}

	model_.transform_ = transform_;
	model_.Update();
	model_.material_.materialData_->color = color_;
}

void Bullet::Draw(Camera* camera) {
	if (isActive_) {
		model_.Draw( camera);
	}
}

void Bullet::ShotBullet(Vector3 position, Vector3 velocity, uint32_t aliveTime) {
	transform_.translate = position;
	velocity_ = velocity;
	aliveTime_ = aliveTime;
	maxAliveTime_ = aliveTime;
	isActive_ = true;
}

void Bullet::OnCollision() {
	isActive_ = false;
}

bool Bullet::GetIsActive() {
	return isActive_;
}

Vector3 Bullet::GetWorldPosition() {
	return transform_.translate;
}

void Bullet::SetName(const std::string& name) {
	name_ = name;
}
