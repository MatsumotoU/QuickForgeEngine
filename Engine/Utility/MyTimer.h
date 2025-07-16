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

#ifdef _DEBUG
	bool GetIsDrawDebugLog();
	void SetIsDrawDebugLog(bool isDraw);
#endif // _DEBUG

private:
	bool isPlaying_;
	std::chrono::steady_clock::time_point startTime_;
	std::chrono::steady_clock::time_point endTime_;
#ifdef _DEBUG
	bool isDrawDebugLog_;
#endif // _DEBUG
};