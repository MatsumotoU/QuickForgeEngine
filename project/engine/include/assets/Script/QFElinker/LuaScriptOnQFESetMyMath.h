#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::MyLuaMath {
	// MyMath関数をLua状態に設定する
	void LuaScriptOnQFESetMyMath(sol::state* luaState);
}
