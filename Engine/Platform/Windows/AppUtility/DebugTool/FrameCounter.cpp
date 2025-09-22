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

	QFE::EngineGlobalValue::deltaTime = deltaTime_;
	QFE::EngineGlobalValue::fps = fps_;
}
