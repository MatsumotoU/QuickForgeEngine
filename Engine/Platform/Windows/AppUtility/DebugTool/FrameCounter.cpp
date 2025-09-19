#include "FrameCounter.h"
#include "Core/EngineGlobalValue.h"

void FrameCounter::Initialize() {
	frameCount_ = 0;
	fps_ = 0.0f;
	deltaTime_ = 0.0f;
	maxFps_ = 60.0f;
}

void FrameCounter::FrameStart() {
	startTime_ = std::chrono::high_resolution_clock::now();
}

void FrameCounter::FrameEnd() {
	endTime_ = std::chrono::high_resolution_clock::now();
	frameCount_++;
	std::chrono::duration<float> elapsedTime = endTime_ - startTime_;
	deltaTime_ = elapsedTime.count();
	if (deltaTime_ > 0.0f) {
		fps_ = 1.0f / deltaTime_;
	} else {
		fps_ = 0.0f;
	}

	// FPS上限制御
	if (maxFps_ <= 0.0f) {
		maxFps_ = 60.0f; // 無効な値を防ぐ
	}

	float targetFrameTime = 1.0f / maxFps_;
	if (deltaTime_ < targetFrameTime) {
		auto sleepDuration = std::chrono::duration<float>(targetFrameTime - deltaTime_);
		std::this_thread::sleep_for(sleepDuration);
		// スリープ後の正確なdeltaTime再計算
		endTime_ = std::chrono::high_resolution_clock::now();
		elapsedTime = endTime_ - startTime_;
		deltaTime_ = elapsedTime.count();
		fps_ = 1.0f / deltaTime_;
	}

	QFE::EngineGlobalValue::deltaTime = deltaTime_;
	QFE::EngineGlobalValue::fps = fps_;
}
