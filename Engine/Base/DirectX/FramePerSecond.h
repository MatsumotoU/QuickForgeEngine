#pragma once
#include <chrono>

const uint32_t kSamplers = 64;

class FramePerSecond {
public:
	void Initialize();
	void Update();

public:
	float GetFps();
	float GetAverageFps();

private:
	float fps_;
	std::chrono::steady_clock::time_point reference_;

	uint32_t listIndex_;
	float fpsList_[kSamplers];
};