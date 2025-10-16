#include "MultiThreadTaskExecutor.h"
#include <vector>

void MultiThreadTaskExecutor::Initialize() {
	nextTaskId_ = 0;
	taskMap_.clear();
}

void MultiThreadTaskExecutor::FrameEnd() {
	// 完了したタスクのIDを収集
	std::vector<uint32_t> completedTasks;
	for (auto& [id, task] : taskMap_) {
		if (task.IsSuccess()) {
			completedTasks.push_back(id);
		}
	}
	// 完了したタスクを削除
	for (auto id : completedTasks) {
		taskMap_.erase(id);
	}
}

void MultiThreadTaskExecutor::Finalize() {
	for (auto& [id, task] : taskMap_) {
		if (task.IsStarted()) {
			while (task.IsRunning()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		task.Init();
	}
	taskMap_.clear();
}

uint32_t MultiThreadTaskExecutor::AddTask(const std::function<void()>& task) {
	uint32_t handle = nextTaskId_++;
	taskMap_[handle].Init();
	taskMap_[handle].Start(task);
	return handle;
}