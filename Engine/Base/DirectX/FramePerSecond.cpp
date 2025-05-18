#include "FramePerSecond.h"

void FramePerSecond::Initialize() {
	reference_ = std::chrono::steady_clock::now();
}

void FramePerSecond::Update() {
	// fpsの算出
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - reference_);

	if (static_cast<float>(elapsed.count()) != 0.0f) {
		fps_ = 1000.0f / static_cast<float>(elapsed.count());
	}
	reference_ = std::chrono::steady_clock::now();
}

float FramePerSecond::GetFps() {
	return fps_;
}
