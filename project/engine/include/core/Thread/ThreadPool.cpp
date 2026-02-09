#include "ThreadPool.h"

QFE::ThreadPool::ThreadPool() {
	size_t threadCount = std::thread::hardware_concurrency();
	if (threadCount > 1) {
		threadCount -= 1; // メインスレッドを残す
	} else {
		threadCount = 1; // 最低1スレッドは確保
	}
	for (size_t i = 0; i < threadCount; ++i) {
		workers_.emplace_back(std::make_unique<ThreadWorker>());
	}
	nextWorkerIndex_ = 0;
}

bool QFE::ThreadPool::AddTask(const std::function<void()>& task) {


	return false;
}
