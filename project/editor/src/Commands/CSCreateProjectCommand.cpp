#include "editor/include/Commands/CSCreateProjectCommand.h"
#include "assets/Script/CsharpVirtualEnvironmentOnQFE.h"

CSCreateProjectCommand::CSCreateProjectCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_create" }) { }

void CSCreateProjectCommand::Execute() {
	if (!command_) return;
	for (auto& alias : aliases_) {
		if (strncmp(command_, alias.c_str(), alias.length()) == 0) {
			projectName_ = command_ + alias.length();
			if (projectName_.length() > 0 && projectName_[0] == ' ') {
				projectName_ = projectName_.substr(1);
			}
			cons_.emplace_back("Create C# Project: " + projectName_);
			CsharpVirtualEnvironmentOnQFE::GetInstance()->CreateCSProject(projectName_);
			return;
		}
	}
}

void CSCreateProjectCommand::Undo() {
}

const std::vector<std::string> CSCreateProjectCommand::GetAliases() const {
	return aliases_;
}
