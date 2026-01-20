#include "editor/include/Commands/LuaScriptInitializeCommand.h"
#include "assets/Script/LuaScriptResourceManager.h"

LuaScriptInitializeCommand::LuaScriptInitializeCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({"lua_init" }) { }

void LuaScriptInitializeCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Run Init All Scripts.");
			LuaScriptResourceManager::GetInstance()->InitializeAllScripts();
			return;
		}
	}
}

void LuaScriptInitializeCommand::Undo() {
}

const std::vector<std::string> LuaScriptInitializeCommand::GetAliases() const {
	return aliases_;
}
