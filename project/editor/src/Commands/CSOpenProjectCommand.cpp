#include "editor/include/Commands/CSOpenProjectCommand.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/CsharpScriptExecutor.h"

using namespace QFE;
CSOpenProjectCommand::CSOpenProjectCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_open" }) { }

void CSOpenProjectCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Open C# ScriptProject");

			// TODO: Uitityに開くコマンドを追加する
			//SceneManager::GetInstance()->GetCsharpScriptExecutor()->OpenCSharpProjectInVSCode();
			return;
		}
	}
}

void CSOpenProjectCommand::Undo() {
}

const std::vector<std::string> CSOpenProjectCommand::GetAliases() const {
	return aliases_;
}
