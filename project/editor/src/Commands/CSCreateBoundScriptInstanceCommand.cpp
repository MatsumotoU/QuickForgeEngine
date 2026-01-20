#include "editor/include/Commands/CSCreateBoundScriptInstanceCommand.h"
#include "assets/Script/CsharpVirtualEnvironmentOnQFE.h"

CSCreateBoundScriptInstanceCommand::CSCreateBoundScriptInstanceCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_cie" }),
	className_("null"),
	entityId_(0){}

void CSCreateBoundScriptInstanceCommand::Execute() {
	if (!command_) return;
	for (auto& alias : aliases_) {
		if (strncmp(command_, alias.c_str(), alias.length()) == 0) {
			std::string args = command_ + alias.length();
			if (args.length() > 0 && args[0] == ' ') {
				args = args.substr(1);
			}
			size_t spacePos = args.find_first_of(' ');
			if (spacePos != std::string::npos) {
				std::string entityIdStr = args.substr(0, spacePos);
				className_ = args.substr(spacePos + 1);
				entityId_ = std::stoi(entityIdStr);
				cons_.emplace_back("Create C# Script Instance: " + className_ + " BindEntity: " + std::to_string(entityId_));
				CsharpVirtualEnvironmentOnQFE::GetInstance()->CreateScriptInstance(entityId_, className_);
			}
			return;
		}
	}
}

void CSCreateBoundScriptInstanceCommand::Undo() {
}

const std::vector<std::string> CSCreateBoundScriptInstanceCommand::GetAliases() const {
	return aliases_;
}
