#include "editor/include/Commands/CSReloadAssemblyCommand.h"
#include "assets/Script/CsharpVirtualEnvironmentOnQFE.h"
using namespace QFE;
CSReloadAssemblyCommand::CSReloadAssemblyCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_reload" }) { }

void CSReloadAssemblyCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Reload C# Assembly");
			CsharpVirtualEnvironmentOnQFE::GetInstance()->ReloadAssembly();
			return;
		}
	}
}

void CSReloadAssemblyCommand::Undo() {
}

const std::vector<std::string> CSReloadAssemblyCommand::GetAliases() const {
	return aliases_;
}
