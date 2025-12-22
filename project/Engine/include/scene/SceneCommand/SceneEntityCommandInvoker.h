#pragma once
#include "ISceneEntityCommand.h"
#include <queue>
#include <memory>
class SceneEntityCommandInvoker final{
private:
	std::queue<std::unique_ptr<ISceneEntityCommand>> commandQueue_;
public:
	SceneEntityCommandInvoker() = default;
	~SceneEntityCommandInvoker() = default;

	void AddCommand(std::unique_ptr<ISceneEntityCommand> command);
	void ExecuteCommands();
	void ClearCommands();
};