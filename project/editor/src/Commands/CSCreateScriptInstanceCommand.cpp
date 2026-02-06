#include "editor/include/Commands/CSCreateScriptInstanceCommand.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"

using namespace QFE;
CSCreateScriptInstanceCommand::CSCreateScriptInstanceCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_ci" }) { }

void CSCreateScriptInstanceCommand::Execute() {
	if (!command_) return;
	for (auto& alias : aliases_) {
		if (strncmp(command_, alias.c_str(), alias.length()) == 0) {
			className_ = command_ + alias.length();
			if (className_.length() > 0 && className_[0] == ' ') {
				className_ = className_.substr(1);
			}
			cons_.emplace_back("Create C# Script Instance: " + className_);
			SceneManager::GetInstance()->GetCsharpScriptExecutor()->CreateScriptInstance(className_);
			return;
		}
	}
}

void CSCreateScriptInstanceCommand::Undo() {
}

const std::vector<std::string> CSCreateScriptInstanceCommand::GetAliases() const {
	return aliases_;
}
