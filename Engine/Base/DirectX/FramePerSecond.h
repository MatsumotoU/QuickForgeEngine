#pragma once
#include <chrono>

class FramePerSecond {
public:
	void Initialize();
	void Update();

public:
	float GetFps();

private:
	float fps_;
	std::chrono::steady_clock::time_point reference_;
};