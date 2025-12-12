#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Base {
	// Inputなどのサブモジュール関数をLua状態にセットする
	void LuaScriptOnQFESetSubModuleBase(sol::state* luaState);
}