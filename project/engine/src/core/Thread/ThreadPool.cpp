#include "engine/include/core/Thread/ThreadPool.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

QFE::ThreadPool::ThreadPool() {
	size_t threadCount = std::thread::hardware_concurrency();
#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("Hardware Concurrency: " + std::to_string(threadCount));
#endif // QFE_OPTIMIZE_OFF

	if (threadCount > 1) {
		threadCount -= 1; // メインスレッドを残す
	} else {
		threadCount = 1; // 最低1スレッドは確保
	}
	for (size_t i = 0; i < threadCount; ++i) {
		workers_.emplace_back(std::make_unique<ThreadWorker>());
	}
}

bool QFE::ThreadPool::AddTask(const std::function<void()>& task) {
	for (auto& worker : workers_) {
		if (!worker->IsBusy()) {
			worker->AddTask(task);
			return true;
		}
	}
	return false;
}

bool QFE::ThreadPool::HasIdleThread() const {
	for (const auto& worker : workers_) {
		if (!worker->IsBusy()) {
			return true;
		}
	}
	return false;
}

size_t QFE::ThreadPool::GetThreadCount() const {
	return workers_.size();
}
