#pragma once
#include <chrono>
#include <cstdint>
#include <thread>
#include <queue>

class FrameCounter final {
public:
	void Initialize();
	void FrameStart();
	void FrameEnd();

	float GetFps() const { return fps_; }
	float GetDeltaTime() const { return deltaTime_; }

private:
	float maxFps_;
	std::chrono::high_resolution_clock::time_point startTime_;
	std::chrono::high_resolution_clock::time_point endTime_;
	uint32_t frameCount_;
	float fps_;
	float deltaTime_;
	std::queue<float> deltaTimeBuffer_;
};