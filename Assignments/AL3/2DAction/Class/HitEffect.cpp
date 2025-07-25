#include "HitEffect.h"
#include <random>

void HitEffect::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;
	for (int i = 0; i < 3; i++) {
		billcoad[i].Initialize(engineCore, 2.0f, 2.0f);
	}
	textureHandle_ = engineCore_->GetTextureManager()->LoadTexture("Resources/point.png");

	isActive_ = false;

	effectDuration_ = 1.0f; // エフェクトの持続時間
	effectElapsedTime_ = 0.0f; // 経過時間

	billcoad[1].transform_.scale.x = 0.3f;
	billcoad[2].transform_.scale.x = 0.3f;
	billcoad[1].transform_.scale.y = 5.0f;
	billcoad[2].transform_.scale.y = 5.0f;

	float r = static_cast<float>(rand() %628 + 1) * 0.01f;
	billcoad[1].transform_.rotate.z = r * 2.0f; // 0から2πの範囲でランダムな角度を設定
	billcoad[2].transform_.rotate.z = r * 3.0f; // 0から2πの範囲でランダムな角度を設定
}

void HitEffect::Update() {
	if (!isActive_) {
		return;
	}

	effectElapsedTime_ += engineCore_->GetDeltaTime();
	if (effectElapsedTime_ >= effectDuration_) {
		isActive_ = false; // エフェクトの持続時間が経過したら非アクティブにする
		effectElapsedTime_ = 0.0f; // 経過時間をリセット
		return;
	}

	billcoad[0].transform_.scale.x = std::powf(std::sinf(3.14f * (effectElapsedTime_ / effectDuration_)), 2.0f) * 2.0f+1.0f;
	billcoad[0].transform_.scale.y = std::powf(std::sinf(3.14f * (effectElapsedTime_ / effectDuration_)), 2.0f) * 2.0f+1.0f;
	
}

void HitEffect::Draw(Camera* camera) {
	if (!isActive_) {
		return;
	}
	for (int i = 0; i < 3; i++) {
		billcoad[i].Update(camera->transform_.rotate);
		billcoad[i].Draw(textureHandle_, camera);
	}
	
}

void HitEffect::EmmitEffect(const Vector3& pos) {
	if (!isActive_) {
		transform_.translate = pos;
		transform_.translate.z += -1.0f;
		for (int i = 0; i < 3; i++) {
			billcoad[i].transform_.translate = transform_.translate;
		}
		isActive_ = true;
	}
}
