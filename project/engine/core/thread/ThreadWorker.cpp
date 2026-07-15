#include "ThreadWorker.h"

QFE::ThreadWorker::ThreadWorker() {
	// スレッドを起動
	isRunning_ = true;
	workerThread_ = std::thread([this]() {
		while (isRunning_) {
			// 排他的処理
			std::unique_lock<std::mutex> lock(mutex_);
			// タスクがあるか、作業者が走っていなければ待機(waitがtrueだったら通過)
			condition_.wait(lock, [this]() { return hasTask_.load() || !isRunning_;}); 

			// 停止要求でタスクを実行せずにループを抜ける
			if (!isRunning_) {
				break;
			}

			// タスクがある場合は実行
			if (hasTask_) {
				// タスクを実行
				if (currentTask_) {
					currentTask_();
				}
				hasTask_ = false;
			}
		}
		});
}

QFE::ThreadWorker::~ThreadWorker() {
	// スレッドを停止
	isRunning_ = false;
	condition_.notify_all(); // 待機中のスレッドを起こす
	if (workerThread_.joinable()) {
		workerThread_.join();
	}
}

void QFE::ThreadWorker::AddTask(const std::function<void()>& task) {
	{
		std::lock_guard<std::mutex> lock(mutex_);
		currentTask_ = task;
		hasTask_ = true;
	}
	condition_.notify_one(); // 待機中のスレッドを起こす
}

bool QFE::ThreadWorker::IsBusy() const {
	return hasTask_;
}