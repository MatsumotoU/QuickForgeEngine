#pragma once
#include <sol/sol.hpp>

namespace QFE {
	class EntityManager;
	class LuaScriptExecutor;
}

namespace QFE::Script {
	// QFE用関数群をLuaの状態に設定する
	void SetQFEFunctions(sol::state* luaState, EntityManager* entityManager, LuaScriptExecutor* luaScriptExecutor);
}
