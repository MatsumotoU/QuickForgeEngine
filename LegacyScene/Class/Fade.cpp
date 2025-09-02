#include "Fade.h"

Fade::Fade(EngineCore* engineCore) :
	engineCore_(engineCore),
	sprite_(engineCore,&camera_, static_cast<float>(engineCore->GetWinApp()->kWindowWidth),
		static_cast<float>(engineCore->GetWinApp()->kWindowHeight)) {}

void Fade::Initialize() {

	/*sprite_.Initialize(engineCore, 
		);*/
	uvCheckerTextureHandle_ = engineCore_->GetTextureManager()->LoadTexture("Resources/uvChecker.png");

	isFadeIn_ = true;
	isFadeOut_ = false;
	fadeTime_ = 0.0f;

	isEndFadeIn_ = false;
	isEndFadeOut_ = false;

	camera_.Initialize(engineCore_->GetWinApp());
}

void Fade::Update() {
	sprite_.material_.GetData()->color = color_;
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
	sprite_.Draw();
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
