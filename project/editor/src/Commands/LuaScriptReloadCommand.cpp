#include "editor/include/Commands/LuaScriptReloadCommand.h"
#include "assets/Script/LuaScriptResourceManager.h"
using namespace QFE;
LuaScriptReloadCommand::LuaScriptReloadCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "lua_reload" }) { }

void LuaScriptReloadCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Reload All Scripts.");
			LuaScriptResourceManager::GetInstance()->ReloadAllScripts();
			return;
		}
	}
}

void LuaScriptReloadCommand::Undo() {
}

const std::vector<std::string> LuaScriptReloadCommand::GetAliases() const {
	return aliases_;
}
