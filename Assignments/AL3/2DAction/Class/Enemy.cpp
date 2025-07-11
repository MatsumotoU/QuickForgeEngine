#include "Enemy.h"

void Enemy::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	transform_.rotate.y = 3.14f / 2.0f + 3.14f;
	model_.material_.materialData_->color = { 1.0f,0.0f,0.0f,1.0f };
	transform_.scale = { 2.0f,2.0f,2.0f };
	frameCount_ = 0.0f;
}

void Enemy::Update() {
	frameCount_++;

	transform_.rotate.z = sinf(frameCount_ * 0.2f) * 0.5f;

	transform_.translate.x -= engineCore_->GetDeltaTime();

	model_.transform_ = transform_;
	model_.Update();
}

void Enemy::Draw(Camera* camera) {
	model_.Draw(camera);
}
