#include "editor/include/Commands/CSCompileScriptCommand.h"
#include "engine/include/utility/script/ScriptUtility.h"

using namespace QFE;
CSCompileScriptCommand::CSCompileScriptCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "cs_compile" }) {
}

void CSCompileScriptCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Compile C# Scripts");
			QFE::Script::CompileScripts();
			return;
		}
	}
}

void CSCompileScriptCommand::Undo() {
	// No undo operation
}

const std::vector<std::string> CSCompileScriptCommand::GetAliases() const {
	return aliases_;
}
