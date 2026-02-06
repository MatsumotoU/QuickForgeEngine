#include "editor/include/Commands/LuaScriptUpdateCommand.h"
#include "engine/include/scene/SceneManager.h"
#include "engine/include/assets/Script/LuaScriptExecutor.h"

using namespace QFE;
LuaScriptUpdateCommand::LuaScriptUpdateCommand(std::vector<std::string>& consoleLog, const char* command)
	: IEditorCommand(consoleLog, command),
	aliases_({ "lua_update" }) { }

void LuaScriptUpdateCommand::Execute() {
	for (auto& alias : aliases_) {
		if (strcmp(command_, alias.c_str()) == 0) {
			cons_.emplace_back("Run Update All Scripts OneFrame.");
			SceneManager::GetInstance()->GetLuaScriptExecutor()->UpdateAllScripts();
			return;
		}
	}
}

void LuaScriptUpdateCommand::Undo() {
}

const std::vector<std::string> LuaScriptUpdateCommand::GetAliases() const {
	return aliases_;
}
