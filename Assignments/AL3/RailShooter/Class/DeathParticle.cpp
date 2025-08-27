#include "DeathParticle.h"
#include <numbers>

void DeathParticle::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	isActive_ = false;
	for (int i = 0; i < kParticles; ++i) {
		model_[i].Initialize(engineCore_);
		model_[i].LoadModel("Resources", "Cube.obj", COORDINATESYSTEM_HAND_RIGHT);

		moveDir[i].x = cosf(std::numbers::pi_v<float> *2.0f * static_cast<float>(i + 1) / static_cast<float>(kParticles));
		moveDir[i].y = sinf(std::numbers::pi_v<float> *2.0f * static_cast<float>(i + 1) / static_cast<float>(kParticles));
	}
	emmitFrame_ = 0;
}

void DeathParticle::Update() {
	if (isActive_) {
		for (int i = 0; i < kParticles; ++i) {
			particleTransform[i].translate +=  moveDir[i] * engineCore_->GetDeltaTime();
			particleTransform[i].rotate += moveDir[i] * engineCore_->GetDeltaTime();
			Vector3 scale = { 1.0f,1.0f,1.0f };
			particleTransform[i].scale = scale * (static_cast<float>(emmitFrame_) / static_cast<float>(kMaxFrame));

			Vector4 color = { 1.0f,1.0f,1.0f,powf(static_cast<float>(emmitFrame_) / static_cast<float>(kMaxFrame),2.0f) };
			model_[i].material_.materialData_->color = color;

			model_[i].transform_ = particleTransform[i];
			model_[i].Update();
		}
	}

	emmitFrame_--;
	if (emmitFrame_ <= 0) {
		isActive_ = false;
	}
}

void DeathParticle::Draw(Camera* camera) {
	if (isActive_) {
		for (int i = 0; i < kParticles; ++i) {
			model_[i].Draw(camera);
		}
	}
}

void DeathParticle::EmmitParticle(const Vector3& pos) {
	if (!isActive_) {
		isActive_ = true;
		for (int i = 0; i < kParticles; ++i) {
			particleTransform[i].translate = pos;
		}
		emmitFrame_ = kMaxFrame;
	}
}

bool DeathParticle::GetIsActive() {
	return isActive_;
}
