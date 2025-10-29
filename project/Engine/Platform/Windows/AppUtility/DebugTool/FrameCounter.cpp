#include "FrameCounter.h"
#include "Core/EngineGlobalValue.h"

#include <windows.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib") 

// TODO: FPS制御を60に戻す

namespace {
	const std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / 60.0f));
	const std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / 65.0f));
}

void FrameCounter::Initialize() {
	frameCount_ = 0;
	fps_ = 0.0f;
	deltaTime_ = 0.0f;
	maxFps_ = 60.0f;
	timeBeginPeriod(1);
}

void FrameCounter::FrameStart() {
	startTime_ = std::chrono::high_resolution_clock::now();
}

void FrameCounter::FrameEnd() {
	endTime_ = std::chrono::high_resolution_clock::now();
	frameCount_++;
	std::chrono::duration<float> elapsedTime = endTime_ - startTime_;

	// FPS下限制御
	if (maxFps_ <= 0.0f) {
		maxFps_ = 30.0f; // 無効な値を防ぐ
	}
	// 60fpsで固定
	while (std::chrono::high_resolution_clock::now() - startTime_ < kMinTime) {
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
	endTime_ = std::chrono::high_resolution_clock::now();
	elapsedTime = endTime_ - startTime_;
	deltaTime_ = elapsedTime.count();

	// FPS計算
	if (deltaTime_ > 0.0f) {
		fps_ = 1.0f / deltaTime_;
	} else {
		fps_ = 0.0f;
	}

	// deltaTime平滑化
	deltaTimeBuffer_.push(deltaTime_);
	if (deltaTimeBuffer_.size() > 512) {
		deltaTimeBuffer_.pop();
	}
	float totalDeltaTime = 0.0f;
	std::queue<float> tempQueue = deltaTimeBuffer_;
	while (!tempQueue.empty()) {
		totalDeltaTime += tempQueue.front();
		tempQueue.pop();
	}
	deltaTime_ = totalDeltaTime / static_cast<float>(deltaTimeBuffer_.size());

	QFE::EngineGlobalValue::deltaTime = deltaTime_;
	QFE::EngineGlobalValue::fps = fps_;
}
