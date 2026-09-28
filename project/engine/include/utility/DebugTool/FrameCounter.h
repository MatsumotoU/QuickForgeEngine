#pragma once
#include <chrono>
#include <cstdint>
#include <thread>

class FrameCounter final {
public:
	void Initialize();
	void FrameStart();
	void FrameEnd();

	float GetFps() const { return fps_; }

private:
	float maxFps_;
	std::chrono::high_resolution_clock::time_point startTime_;
	std::chrono::high_resolution_clock::time_point endTime_;
	uint32_t frameCount_;
	float fps_;
};
