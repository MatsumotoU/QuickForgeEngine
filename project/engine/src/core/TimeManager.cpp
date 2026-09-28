#include "engine/include/core/TimeManager.h"

void TimeManager::Initialize() {
	deltaTimeSamples_ = {};
	unscaledDeltaTime_ = 0.0f;
	deltaTime_ = 0.0f;
	hitStopRemaining_ = 0.0f;
}

void TimeManager::Update(float unscaledDeltaTime) {
	if (unscaledDeltaTime < 0.0f) {
		unscaledDeltaTime = 0.0f;
	}

	deltaTimeSamples_.push(unscaledDeltaTime);
	if (deltaTimeSamples_.size() > kDeltaTimeSampleCount) {
		deltaTimeSamples_.pop();
	}

	float totalDeltaTime = 0.0f;
	std::queue<float> samples = deltaTimeSamples_;
	while (!samples.empty()) {
		totalDeltaTime += samples.front();
		samples.pop();
	}
	unscaledDeltaTime_ = totalDeltaTime / static_cast<float>(deltaTimeSamples_.size());

	if (hitStopRemaining_ > 0.0f) {
		deltaTime_ = 0.0f;
		hitStopRemaining_ -= unscaledDeltaTime_;
		if (hitStopRemaining_ < 0.0f) {
			hitStopRemaining_ = 0.0f;
		}
	} else {
		deltaTime_ = unscaledDeltaTime_;
	}
}

void TimeManager::StartHitStop(float duration) {
	if (duration > 0.0f) {
		hitStopRemaining_ = duration;
	}
}
