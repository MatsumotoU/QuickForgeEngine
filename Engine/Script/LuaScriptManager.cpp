#include "LuaScriptManager.h"
#pragma comment(lib, "Lua.lib")

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

LuaScriptManager::LuaScriptManager() {
    lua_.open_libraries(sol::lib::base, sol::lib::math, sol::lib::package);
}

LuaScriptManager::~LuaScriptManager() {}

sol::state& LuaScriptManager::GetLuaState() {
    return lua_;
}

void LuaScriptManager::BindDebugLog() {
	// Luaの関数をC++の関数にバインド
    lua_.set_function("DebugLog", [](const std::string& msg) {
        DebugLog(msg);
        });
}
