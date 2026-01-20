#include "editor/include/Commands/CSOpenProjectCommand.h"
#include "assets/Script/CsharpVirtualEnvironmentOnQFE.h"

CSOpenProjectCommand::CSOpenProjectCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_open" }) { }

void CSOpenProjectCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Open C# ScriptProject");
			CsharpVirtualEnvironmentOnQFE::GetInstance()->OpenCSharpProjectInVSCode();
			return;
		}
	}
}

void CSOpenProjectCommand::Undo() {
}

const std::vector<std::string> CSOpenProjectCommand::GetAliases() const {
	return aliases_;
}
