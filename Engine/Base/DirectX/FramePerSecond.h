#pragma once
#include <chrono>

const uint32_t kFpsSamplers = 64;

class FramePerSecond final {
public:
	void Initialize();
	void Update();

public:
	float GetFps();
	float GetAverageFps();
	float* GetFpsSample();
	uint32_t GetFpsSamplerNum();

private:
	float fps_;
	std::chrono::steady_clock::time_point reference_;

	uint32_t listIndex_;
	float fpsList_[kFpsSamplers];
};