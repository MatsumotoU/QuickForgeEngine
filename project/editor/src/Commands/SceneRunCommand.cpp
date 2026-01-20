#include "editor/include/Commands/SceneRunCommand.h"
#include "scene/SceneManager.h"

SceneRunCommand::SceneRunCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "scene_run" }) { }

void SceneRunCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Run Scene.");
			SceneManager::GetInstance()->StartScript();
			return;
		}
	}
}

void SceneRunCommand::Undo() {
}

const std::vector<std::string> SceneRunCommand::GetAliases() const {
	return aliases_;
}
