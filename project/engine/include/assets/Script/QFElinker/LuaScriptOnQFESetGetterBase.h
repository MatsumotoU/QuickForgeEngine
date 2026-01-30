#pragma once
#include <sol/sol.hpp>

namespace QFE {
	class EntityManager;
}

namespace QFE::Script::Base {
	// 変数取得関数をLua状態に設定する
	void LuaScriptOnQFESetGetterBase(sol::state* luaState, EntityManager* entityManager);
}
