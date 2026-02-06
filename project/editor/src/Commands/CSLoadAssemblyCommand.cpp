#include "editor/include/Commands/CSLoadAssemblyCommand.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"

using namespace QFE;
CSLoadAssemblyCommand::CSLoadAssemblyCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_load" }) { }

void CSLoadAssemblyCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Load C# Assembly");
			//SceneManager::GetInstance()->GetCsharpScriptExecutor()->LoadAssembly();
			return;
		}
	}
}

void CSLoadAssemblyCommand::Undo() {
}

const std::vector<std::string> CSLoadAssemblyCommand::GetAliases() const {
	return aliases_;
}
