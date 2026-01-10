#pragma once
#include <sol/sol.hpp>

namespace QFE::Script {
	// QFE用関数群をLuaの状態に設定する
	void SetQFEFunctions(sol::state* luaState);
}
