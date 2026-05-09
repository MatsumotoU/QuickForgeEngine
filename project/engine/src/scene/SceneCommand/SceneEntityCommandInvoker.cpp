#include "engine/include/scene/SceneCommand/SceneEntityCommandInvoker.h"
#include <chrono>
using namespace QFE;

#include "engine/include/core/EngineDefines.h"

QFE::SceneEntityCommandInvoker::SceneEntityCommandInvoker(float commandTimeout) :
	commandTimeout_(commandTimeout) {
}

void QFE::SceneEntityCommandInvoker::AddSystemCommand(std::unique_ptr<ISceneEntityCommand> command) {
	systemCommandQueue_.push(std::move(command));
}

void QFE::SceneEntityCommandInvoker::AddUserCommand(std::unique_ptr<ISceneEntityCommand> command) {
	userCommandQueue_.push(std::move(command));
}

void SceneEntityCommandInvoker::ExecuteCommands() {
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime = std::chrono::high_resolution_clock::now();

	// システム側は絶対に実行される
	while (!systemCommandQueue_.empty()) {
		auto& command = systemCommandQueue_.front();
		command->Execute();
		systemCommandQueue_.pop();
	}

	// ユーザー側はタイムアウトまで実行される
	while (!userCommandQueue_.empty()) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		float elapsedTime = std::chrono::duration<float>(currentTime - startTime).count();
		if (elapsedTime >= commandTimeout_) {
#ifdef QFE_OPTIMIZE_OFF
			QFE_LOG("SceneEntityCommandInvoker: User command execution timed out.", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
			break;
		}
		auto& command = userCommandQueue_.front();
		command->Execute();
		userCommandQueue_.pop();
	}
}

void SceneEntityCommandInvoker::ClearCommands() {
	while (!systemCommandQueue_.empty()) {
		systemCommandQueue_.pop();
	}
	while (!userCommandQueue_.empty()) {
		userCommandQueue_.pop();
	}
}
