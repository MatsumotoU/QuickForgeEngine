#include "engine/include/core/Thread/PriorityTaskDispatcher.h"
#include <algorithm>

QFE::PriorityTaskDispatcher::PriorityTaskDispatcher(
	std::function<bool(const std::function<void()>&)> addTaskFunc,
	std::function<bool()> hasIdleWorker) :
	addTaskFunc_(addTaskFunc),
	hasIdleWorker_(hasIdleWorker) {
}

void QFE::PriorityTaskDispatcher::AddTask(const std::function<void()>& task, int priority) {
	// タスクを追加
	tasks_.push_back({ task, priority });
	// 優先度順にソート
	std::sort(tasks_.begin(), tasks_.end(),
		[](const Task& a, const Task& b) {
			return a.priority < b.priority;
		});
}

void QFE::PriorityTaskDispatcher::ExecuteTasks() {
	while (!tasks_.empty() && hasIdleWorker_()) {
		// 最も優先度の高いタスクを取得
		Task task = tasks_.front();
		tasks_.erase(0);
		// タスクを追加
		addTaskFunc_(task.func);
	}
}
