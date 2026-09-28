#include "engine/include/utility/DebugTool/FrameCounter.h"
#include "engine/include/core/EngineGlobalValue.h"
#include "engine/include/core/TimeManager.h"

#include <windows.h>
#include <timeapi.h>
#pragma comment(lib,"winmm.lib") 

#ifdef _DEBUG
#include "Engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

namespace {
	const std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / 60.0f));
	const std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / 65.0f));
}

void FrameCounter::Initialize() {
	frameCount_ = 0;
	fps_ = 0.0f;
	TimeManager::GetInstance()->Initialize();
	maxFps_ = 60.0f;
	timeBeginPeriod(1);

#ifdef _DEBUG
	DebugLog("FrameCounter Initialized");
#endif // _DEBUG

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
		maxFps_ = 60.0f; // 無効な値を防ぁE
	}
	// 60fpsで固宁E
	while (std::chrono::high_resolution_clock::now() - startTime_ < kMinTime) {
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
	endTime_ = std::chrono::high_resolution_clock::now();
	elapsedTime = endTime_ - startTime_;
	const float unscaledDeltaTime = elapsedTime.count();

	// FPS計箁E
	if (unscaledDeltaTime > 0.0f) {
		fps_ = 1.0f / unscaledDeltaTime;
	} else {
		fps_ = 0.0f;
	}

	TimeManager::GetInstance()->Update(unscaledDeltaTime);
	QFE::EngineGlobalValue::fps = fps_;
}
