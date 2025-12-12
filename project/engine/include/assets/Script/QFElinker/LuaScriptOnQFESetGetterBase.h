#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Base {
	// 変数取得関数をLua状態にセットする
	void LuaScriptOnQFESetGetterBase(sol::state* luaState);
}