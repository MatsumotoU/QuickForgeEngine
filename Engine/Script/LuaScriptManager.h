#pragma once
#include <sol/sol.hpp>

class LuaScriptManager {
public:
    LuaScriptManager();
    ~LuaScriptManager();
    sol::state& GetLuaState();
	void BindDebugLog();
private:
    sol::state lua_;
};