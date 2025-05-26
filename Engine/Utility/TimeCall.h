#pragma once
#include <functional>

class EngineCore;

class TimeCall {
public:
	TimeCall(EngineCore * engineCore,std::function<void()> callBack, float time);
	void Update();
	bool IsFinished();

private:
	std::function<void()> callFunction_;
	float time_;
	bool isFinished_;
	EngineCore* engineCore_;
};