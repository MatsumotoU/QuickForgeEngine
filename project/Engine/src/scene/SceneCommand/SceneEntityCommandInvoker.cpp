#include "engine/include/scene/SceneCommand/SceneEntityCommandInvoker.h"
void SceneEntityCommandInvoker::AddCommand(std::unique_ptr<ISceneEntityCommand> command){
	commandQueue_.push(std::move(command));
}

void SceneEntityCommandInvoker::ExecuteCommands() {
	while (!commandQueue_.empty()) {
		auto& command = commandQueue_.front();
		command->Execute();
		commandQueue_.pop();
	}
}

void SceneEntityCommandInvoker::ClearCommands(){
	while (!commandQueue_.empty()) {
		commandQueue_.pop();
	}
}
