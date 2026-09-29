#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Base {
	// QFE用の基本構造体をLua状態に設定する
	void SetOnQFESetStructBase(sol::state* luaState);
}
