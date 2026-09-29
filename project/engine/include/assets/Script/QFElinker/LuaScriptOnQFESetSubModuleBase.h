#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Base {
	// Inputなどのサブモジュール関数をLua状態に設定する
	void LuaScriptOnQFESetSubModuleBase(sol::state* luaState);
}
