#include "LoopStoper.h"

void LoopStoper::Initialize() {
	nonTimeStoppingFunctions_.clear();
	isStopping_ = false;
}

void LoopStoper::Update() {
	if (isStopping_) {
		for (std::function<void()>& func : nonTimeStoppingFunctions_) {
			func();
		}
	}
}

void LoopStoper::AddNonStoppingFunc(std::function<void()> func) {
	nonTimeStoppingFunctions_.push_back(func);
}

bool LoopStoper::GetIsStopping() {
	return isStopping_;
}

void LoopStoper::SetIsStopping(bool set) {
	isStopping_ = set;
}
