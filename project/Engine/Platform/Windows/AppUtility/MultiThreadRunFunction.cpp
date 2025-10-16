#include "MultiThreadRunFunction.h"

MultiThreadRunFunction::MultiThreadRunFunction() : isRunning(false), isStarted(false) {}

MultiThreadRunFunction::~MultiThreadRunFunction() {
	if (t.joinable()) {
		t.join();
	}
}

void MultiThreadRunFunction::Init() {
	isRunning = false;
	isStarted = false;
	if (t.joinable()) {
		t.join();
	}
}

void MultiThreadRunFunction::Start(std::function<void()> func) {
	if (isRunning || isStarted) {
		return;
	}

	isRunning = true;
	isStarted = true;
	t = std::thread([this, func]() {
		func();
		isRunning = false;
		});
}

const bool MultiThreadRunFunction::IsStarted() const {
	return isStarted;
}

const bool MultiThreadRunFunction::IsRunning() const {
	return isRunning;
}

const bool MultiThreadRunFunction::IsSuccess() const {
	return (!isRunning && isStarted);
}
