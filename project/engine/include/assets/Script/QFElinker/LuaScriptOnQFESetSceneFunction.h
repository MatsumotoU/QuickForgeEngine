#pragma once
#include <sol/sol.hpp>

namespace QFE::Script::Scene {
	// シーン操作関数をLua状態にセチE��する
	void LuaScriptOnQFESetSceneFunction(sol::state* luaState);
}
