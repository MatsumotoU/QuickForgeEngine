#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Base {
	// QFE用の基本構造体をLua状態にセットする
	void SetOnQFESetStructBase(sol::state* luaState);
}