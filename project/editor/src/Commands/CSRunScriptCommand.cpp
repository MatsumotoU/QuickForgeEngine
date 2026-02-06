#include "editor/include/Commands/CSRunScriptCommand.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"

using namespace QFE;
CSRunScriptCommand::CSRunScriptCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_run" }) {
}

void CSRunScriptCommand::Execute() {
	if (!command_) return;
	for (auto& alias : aliases_) {
		if (strncmp(command_, alias.c_str(), alias.length()) == 0) {
			std::string args = command_ + alias.length();
			if (args.length() > 0 && args[0] == ' ') {
				args = args.substr(1);
			}
			size_t spacePos = args.find_first_of(' ');
			if (spacePos != std::string::npos) {
				std::string indexStr = args.substr(0, spacePos);
				functionName_ = args.substr(spacePos + 1);
				index_ = std::stoi(indexStr);
				cons_.emplace_back("Run C# Script Function: " + functionName_ + " on Instance Index: " + std::to_string(index_));
				SceneManager::GetInstance()->GetCsharpScriptExecutor()->RunScriptFunction(index_, functionName_);
			}
			return;
		}
	}
}

void CSRunScriptCommand::Undo() {
}

const std::vector<std::string> CSRunScriptCommand::GetAliases() const {
	return aliases_;
}
