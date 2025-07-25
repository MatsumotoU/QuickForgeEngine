#include "Enemy.h"

void Enemy::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	model_.Initialize(engineCore_);
	model_.LoadModel("Resources", "Speaker.obj", COORDINATESYSTEM_HAND_RIGHT);

	transform_.rotate.y = 3.14f / 2.0f + 3.14f;
	model_.material_.materialData_->color = { 1.0f,0.0f,0.0f,1.0f };
	transform_.scale = { 2.0f,2.0f,2.0f };
	frameCount_ = 0.0f;

	isAlive_ = true;

	// 行動を定義する
	bahaviorTable_.clear();
	bahaviorTable_.emplace(EnemyBahaviorType::kMove, std::bind(&Enemy::Move, this));
	bahaviorTable_.emplace(EnemyBahaviorType::kDeath, std::bind(&Enemy::Death, this));
	bahaviorType_ = EnemyBahaviorType::kMove;
}

void Enemy::Update() {
	frameCount_++;

	bahaviorTable_.at(bahaviorType_)();

	model_.transform_ = transform_;
	model_.Update();
}

void Enemy::Draw(Camera* camera) {
	if (!isAlive_) {
		if (deathAnimationFrameCount_ <= 0) {
			return; // 死亡アニメーションが終了したら描画しない
		}
	}

	model_.Draw(camera);
}

void Enemy::Move() {
	transform_.rotate.z = sinf(frameCount_ * 0.2f) * 0.5f;
	transform_.translate.x -= engineCore_->GetDeltaTime();
}

void Enemy::Death() {
	if (isAlive_) {
		isAlive_ = false;
		deathAnimationFrameCount_ = 60;
	}

	if (deathAnimationFrameCount_ > 0) {
		deathAnimationFrameCount_--;
		transform_.rotate.z += 3.5f * engineCore_->GetDeltaTime();
	}
}
