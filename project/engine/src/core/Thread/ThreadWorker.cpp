#include "engine/include/core/Thread/ThreadWorker.h"

QFE::ThreadWorker::ThreadWorker() {
	// スレッドを起動
	isRunning_ = true;
	workerThread_ = std::thread([this]() {
		while (isRunning_) {
			if (hasTask_) {
				// タスクを実行
				if (currentTask_) {
					currentTask_();
				}
				hasTask_ = false;
			}
			std::this_thread::yield(); // 他のスレッドに実行を譲る
		}
		});
}

QFE::ThreadWorker::~ThreadWorker() {
	// スレッドを停止
	isRunning_ = false;
	if (workerThread_.joinable()) {
		workerThread_.join();
	}
}

void QFE::ThreadWorker::AddTask(const std::function<void()>& task) {
	currentTask_ = task;
	hasTask_ = true;
}

bool QFE::ThreadWorker::IsBusy() const {
	return hasTask_;
}
