#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Utility {
	// ユーティリティ関数をLua状態に設定する
	void LuaScriptOnQFESetUtility(sol::state* luaState);
}
