#pragma once
#include <sol/sol.hpp>
#include <string>

class ILuaScript {
public:
	ILuaScript();
	virtual ~ILuaScript() = default;
	sol::state& GetLuaState();

protected:
	sol::state luaState_;
};