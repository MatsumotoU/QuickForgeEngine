#include "TimeCall.h"
#include "../Base/EngineCore.h"

TimeCall::TimeCall(EngineCore* engineCore, std::function<void()> callBack, float time):
	engineCore_(engineCore), callFunction_(callBack), time_(time), isFinished_(false) { }

void TimeCall::Update() {
	if (isFinished_) {
		return;
	}

	time_ -= engineCore_->GetDeltaTime();

	if (time_ <= 0) {
		isFinished_ = true;
		callFunction_();
	}

}

bool TimeCall::IsFinished() {
	return isFinished_;
}
