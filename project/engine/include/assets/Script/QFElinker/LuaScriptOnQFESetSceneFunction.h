#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Scene {
	// シーン操作関数をLua状態に設定する
	void LuaScriptOnQFESetSceneFunction(sol::state* luaState);
}
