#pragma once
#include <chrono>

class MyTimer {
public:
	~MyTimer();

public:
	/// <summary>
	/// タイマーをリセットします
	/// </summary>
	void Init();

public:
	void StartTimer();
	void StopTimer();

public:
	float GetElapsedTime();
	bool GetIsPlaying();

private:
	bool isPlaying_;
	std::chrono::steady_clock::time_point startTime_;
	std::chrono::steady_clock::time_point endTime_;
};