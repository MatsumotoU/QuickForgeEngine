#pragma once
#include <sol/sol.hpp>

namespace QFE::Script {
	// QFE用関数群をLua状態にセットする
	void SetQFEFunctions(sol::state* luaState);
}