#include "TimeCall.h"
#include "../Base/EngineCore.h"

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG


TimeCall::TimeCall(EngineCore* engineCore, std::function<void()> callBack, float time):
	engineCore_(engineCore), callFunction_(callBack), time_(time), isFinished_(false) { 
#ifdef _DEBUG
	DebugLog(std::format("CreateTimeCall Time: {}", time_));
#endif // _DEBUG
}

void TimeCall::Update() {
#ifdef _DEBUG
	DebugLog(std::format("TimeCall CountDown {}", time_));
#endif // _DEBUG

	if (isFinished_) {
		return;
	}

	time_ = time_ - engineCore_->GetDeltaTime();

	if (time_ <= 0) {
		isFinished_ = true;
		callFunction_();
#ifdef _DEBUG
		DebugLog(std::format("TimeCall finished. Time: {}", time_));
#endif // _DEBUG
	}
}

bool TimeCall::IsFinished() {
	return isFinished_;
}
