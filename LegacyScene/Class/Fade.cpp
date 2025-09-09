#include "Fade.h"
#include "Utility/MyEasing.h"

Fade::Fade(EngineCore* engineCore) :
	engineCore_(engineCore) {
}

void Fade::Initialize() {
	uvCheckerTextureHandle_ = engineCore_->GetTextureManager()->LoadTexture("Resources/White1x1.png");

	isFadeIn_ = true;
	isFadeOut_ = false;
	fadeTime_ = 0.0f;

	isEndFadeIn_ = false;
	isEndFadeOut_ = false;

	sprites_.resize(kSpalateNum * kSpalateNum);

	for (size_t i = 0; i < sprites_.size(); ++i) {
		sprites_[i] = std::make_unique<Sprite>(engineCore_, &camera_,
			static_cast<float>(engineCore_->GetWinApp()->kWindowWidth / kSpalateNum),
			static_cast<float>(engineCore_->GetWinApp()->kWindowHeight / kSpalateNum));

		int col = static_cast<int>(i % kSpalateNum);
		int row = static_cast<int>(i / kSpalateNum);

		// 右上から左下へ配置（左上端が画面端に来るように）
		float x = (static_cast<float>(engineCore_->GetWinApp()->kWindowWidth) / kSpalateNum) * (kSpalateNum - 1 - col);
		float y = (static_cast<float>(engineCore_->GetWinApp()->kWindowHeight) / kSpalateNum) * (kSpalateNum - 1 - row);

		sprites_[i]->SetTextureHandle(uvCheckerTextureHandle_);
		sprites_[i]->transform_.translate = { x, y, 0.0f };
	}

	camera_.Initialize(engineCore_->GetWinApp());
}

void Fade::Update() {
	float windowHeight = static_cast<float>(engineCore_->GetWinApp()->kWindowHeight);

	for (auto& sprite : sprites_) {
		// スプライトのY座標取得
		float spriteY = sprite->transform_.translate.y;

		// 下から上への割合（0.0:下, 1.0:上）
		float ratio = spriteY / windowHeight;
		ratio = std::clamp(ratio, 0.0f, 1.0f);

		// フェード進行度
		float fadeProgress = 1.0f - fadeTime_ / kFadeInTime;

		// スプライトごとにフェード進行度をずらす
		float localFade = fadeProgress * kSpalateNum; // kSpalateNum分割で進行
		float threshold = ratio * kSpalateNum;

		// 白への補間率（下ほど早く白くなる）
		float whiteRate = std::clamp(localFade - threshold, 0.0f, 1.0f);

		Vector4 baseColor = color_;
		Vector4 whiteColor = Vector4(1.0f, 1.0f, 1.0f, baseColor.w);
		Vector4 finalColor = baseColor * (1.0f - whiteRate) + whiteColor * whiteRate;

		if (isFadeIn_) {
			MyEasing::SimpleEaseIn(&sprite->transform_.scale.x, 0.0f, 0.5f);
			MyEasing::SimpleEaseIn(&sprite->transform_.scale.y, 0.0f, 0.5f);
		}

		if (isFadeOut_) {
			MyEasing::SimpleEaseIn(&sprite->transform_.scale.x, 1.0f, 0.5f);
			MyEasing::SimpleEaseIn(&sprite->transform_.scale.y, 1.0f, 0.5f);
		}

		sprite->material_.GetData()->color = finalColor;
		sprite->Update();
	}

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
	for (auto& sprite : sprites_) {
		sprite->Draw();
	}
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
