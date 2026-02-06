#include "engine/include/assets/Script/LuaRuntimeManager.h"
#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"
#include "engine/include/core/Entity/EntityManager.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif

using namespace QFE;

void LuaRuntimeManager::Initialize() {
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("LuaRuntimeManager initialized.");
#endif
}

void LuaRuntimeManager::Finalize() {
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("LuaRuntimeManager finalized.");
#endif
}

std::unique_ptr<sol::state> LuaRuntimeManager::CreateLuaState(EntityManager* entityManager, LuaScriptExecutor* luaScriptExecutor) {
	auto luaState = std::make_unique<sol::state>();

	// 標準ライブラリの登録
	RegisterStandardLibraries(luaState.get());

	// QFEという名前のグローバルテーブルを作成
	luaState->create_named_table("QFE");

	// QFE APIの登録
	RegisterQFEAPI(luaState.get(), entityManager, luaScriptExecutor);

	// Lua側の更新リスト管理システムを登録
	luaState->script(R"(
		QFE_Internal = {
			update_list = {},
			UpdateAll = function()
				if QFE_Internal.dirty then
					QFE_Internal.sorted_list = {}
					for handle, entry in pairs(QFE_Internal.update_list) do
						table.insert(QFE_Internal.sorted_list, entry)
					end
					table.sort(QFE_Internal.sorted_list, function(a, b) return a.priority < b.priority end)
					QFE_Internal.dirty = false
				end
				for _, entry in ipairs(QFE_Internal.sorted_list) do
					entry.func()
				end
			end,

			RegisterUpdate = function(handle, func, priority)
				QFE_Internal.update_list[handle] = {func = func, priority = priority}
				QFE_Internal.dirty = true
			end,
			UnregisterUpdate = function(handle)
				if QFE_Internal.update_list[handle] then
					QFE_Internal.update_list[handle] = nil
					QFE_Internal.dirty = true
				end
			end,
			ClearList = function()
				QFE_Internal.update_list = {}
				QFE_Internal.sorted_list = {}
				QFE_Internal.dirty = false
			end,
			dirty = false,
			sorted_list = {}
		}
	)");

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Created new Lua state with QFE API.");
#endif

	return luaState;
}

void LuaRuntimeManager::RegisterStandardLibraries(sol::state* state) {
	state->open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::math,
		sol::lib::string,
		sol::lib::table,
		sol::lib::coroutine,
		sol::lib::debug,
		sol::lib::utf8
	);
}

void LuaRuntimeManager::RegisterQFEAPI(sol::state* state, EntityManager* entityManager, LuaScriptExecutor* luaScriptExecutor) {
	// QFE APIの登録（EntityManagerを使用する関数群）
	Script::SetQFEFunctions(state, entityManager, luaScriptExecutor);
}
