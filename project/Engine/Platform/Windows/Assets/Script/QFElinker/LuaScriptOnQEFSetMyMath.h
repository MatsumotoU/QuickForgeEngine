#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::MyLuaMath {
	// MyMath関数をLua状態にセットする
	void LuaScriptOnQEFSetMyMath(sol::state* luaState);
}