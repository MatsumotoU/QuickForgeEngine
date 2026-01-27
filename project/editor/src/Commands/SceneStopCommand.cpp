#include "editor/include/Commands/SceneStopCommand.h"
#include "scene/SceneManager.h"
using namespace QFE;
SceneStopCommand::SceneStopCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "scene_stop" }) { }

void SceneStopCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Stop Scene.");
			SceneManager::GetInstance()->StopScript();
			return;
		}
	}
}

void SceneStopCommand::Undo() {
}

const std::vector<std::string> SceneStopCommand::GetAliases() const {
	return aliases_;
}
