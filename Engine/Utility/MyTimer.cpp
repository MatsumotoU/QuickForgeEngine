#include "MyTimer.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

MyTimer::~MyTimer() {
    if (isPlaying_) {
#ifdef _DEBUG
        isDrawDebugLog_ = true;
        DebugLog("The timer was removed before it finished.");
#endif // _DEBUG
    }
}

void MyTimer::Init() {
	startTime_ = {};
	endTime_ = {};
}

void MyTimer::StartTimer() {
	isPlaying_ = true;
	startTime_ = std::chrono::steady_clock::now();

#ifdef _DEBUG
    DebugLog("StartTimer");
#endif // _DEBUG
}

void MyTimer::StopTimer() {
	isPlaying_ = false;
	endTime_ = std::chrono::steady_clock::now();

#ifdef _DEBUG
    DebugLog("StartTimer");
#endif // _DEBUG
}

float MyTimer::GetElapsedTime() {
    if (isPlaying_) {
        // 計測中は現在時刻との差分
        auto now = std::chrono::steady_clock::now();
#ifdef _DEBUG
        DebugLog(std::format( "Timer is running! Return: {}", duration_cast<std::chrono::duration<float>>(now - startTime_).count()));
#endif // _DEBUG
        return duration_cast<std::chrono::duration<float>>(now - startTime_).count();
    } else {
        // 停止中はstartとendの差分
#ifdef _DEBUG
        DebugLog(std::format("Return: {}", duration_cast<std::chrono::duration<float>>(endTime_ - startTime_).count()));
#endif // _DEBUG
        return duration_cast<std::chrono::duration<float>>(endTime_ - startTime_).count();
    }
}

bool MyTimer::GetIsPlaying() {
    return isPlaying_;
}

#ifdef _DEBUG
bool MyTimer::GetIsDrawDebugLog() {
    return isDrawDebugLog_;
}

void MyTimer::SetIsDrawDebugLog(bool isDraw) {
	isDrawDebugLog_ = isDraw;
}
#endif // _DEBUG
