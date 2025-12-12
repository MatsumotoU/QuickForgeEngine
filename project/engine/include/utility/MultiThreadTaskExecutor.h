#pragma once
#include "Utility/DesignPatterns/Singleton.h"

#include <unordered_map>
#include "MultiThreadRunFunction.h"
class MultiThreadTaskExecutor final : public Singleton<MultiThreadTaskExecutor> {
	friend class Singleton<MultiThreadTaskExecutor>;
public:
	MultiThreadTaskExecutor() = default;
	~MultiThreadTaskExecutor() = default;
	void Initialize();
	void FrameEnd();
	void Finalize();
	
	// タスクを追加
	uint32_t AddTask(const std::function<void()>& task);

private:
	uint32_t nextTaskId_ = 0;
	std::unordered_map<uint32_t, MultiThreadRunFunction> taskMap_;
};