#include "Enemy.h"

void Enemy::Initialize(EngineCore* engineCore, Camera* camera) {
	engineCore_ = engineCore;
	camera_ = camera;
	model_ = std::make_unique<Model>(engineCore, camera);
	model_->LoadModel("Resources", "Cube.obj", COORDINATESYSTEM_HAND_RIGHT);
}

void Enemy::Update() {
	model_->Update();
}

void Enemy::Draw() {
	model_->Draw();
}