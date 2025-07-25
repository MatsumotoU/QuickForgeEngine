#include "Fade.h"

void Fade::Initialize(EngineCore* engineCore) {
	sprite_.Initialize(engineCore, 
		static_cast<float>(engineCore->GetWinApp()->kWindowWidth),
		static_cast<float>(engineCore->GetWinApp()->kWindowHeight));
	uvCheckerTextureHandle_ = engineCore->GetTextureManager()->LoadTexture("Resources/uvChecker.png");

	isFadeIn_ = true;
	isFadeOut_ = false;
	fadeTime_ = 0.0f;

	isEndFadeIn_ = false;
	isEndFadeOut_ = false;

	camera_.Initialize(engineCore->GetWinApp());
}

void Fade::Update() {
	sprite_.material_.materialData_->color = color_;
	if (isFadeIn_ && !isEndFadeIn_) {
		fadeTime_++;
		if (fadeTime_ >= kFadeInTime) {
			fadeTime_ = kFadeInTime;
			isEndFadeIn_ = true;
			isFadeIn_ = false;
		}
		color_.w = 1.0f - fadeTime_ / kFadeInTime;
	}

	if (isFadeOut_ && !isEndFadeOut_) {
		fadeTime_--;
		if (fadeTime_ <= 0.0f) {
			fadeTime_ = 0.0f;
			isEndFadeOut_ = true;
			isFadeOut_ = false;
		}
		color_.w = 1.0f - fadeTime_ / kFadeInTime;
	}

	
}

void Fade::Draw() {

	ImGui::Text("%f", fadeTime_);

	sprite_.DrawSprite(transform_, uvCheckerTextureHandle_, &camera_);
}

void Fade::StartFadeIn() {
	if (!isFadeIn_) {
		isFadeOut_ = false;
		isEndFadeOut_ = false;

		isFadeIn_ = true;
		isEndFadeIn_ = false;
	}
}

void Fade::StartFadeOut() {
	if (!isFadeOut_) {
		isFadeIn_ = false;
		isEndFadeIn_ = false;

		isFadeOut_ = true;
		isEndFadeOut_ = false;
	}
}

bool Fade::isFading() {
	if (fadeTime_ >= kFadeInTime) {
		return false;
	}
	return true;
}
