#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::MyLuaMath {
	// MyMath関数をLua状態にセチE��する
	void LuaScriptOnQEFSetMyMath(sol::state* luaState);
}
