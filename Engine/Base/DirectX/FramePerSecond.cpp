#include "FramePerSecond.h"

void FramePerSecond::Initialize() {
	reference_ = std::chrono::steady_clock::now();
	listIndex_ = 0;
	for (int i = 0; i < kFpsSamplers; i++) {
		fpsList_[i] = 60.0f;
	}
}

void FramePerSecond::Update() {
	// fpsの算出
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - reference_);

	if (static_cast<float>(elapsed.count()) != 0.0f) {
		fps_ = 1000.0f / static_cast<float>(elapsed.count());
	}
	reference_ = std::chrono::steady_clock::now();

	fpsList_[listIndex_] = fps_;
	listIndex_++;
	if (listIndex_ >= kFpsSamplers) {
		listIndex_ = 0;
	}
}

float FramePerSecond::GetFps() {
	return fps_;
}

float FramePerSecond::GetAverageFps() {
	float result = 0.0f;
	for (int i = 0; i < kFpsSamplers; i++) {
		result += fpsList_[i];
	}
	result /= static_cast<float>(kFpsSamplers);

	return result;
}

float* FramePerSecond::GetFpsSample() {
	return fpsList_;
}

uint32_t FramePerSecond::GetFpsSamplerNum() {
	return kFpsSamplers;
}
