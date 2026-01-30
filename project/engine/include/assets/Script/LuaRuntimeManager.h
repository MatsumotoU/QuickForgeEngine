#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include <sol/sol.hpp>
#include <memory>

namespace QFE {

	class EntityManager;

	/**
	 * @brief Luaランタイムのグローバル管理
	 * 共通ライブラリとAPI登録のテンプレートを管理
	 */
	class LuaRuntimeManager final : public Singleton<LuaRuntimeManager> {
		friend class Singleton<LuaRuntimeManager>;

	public:
		/// @brief 初期化
		void Initialize();

		/// @brief 終了処理
		void Finalize();

		/// @brief 新しいLua stateを作成し、標準ライブラリとQFE APIを登録
		/// @param entityManager このstateで使用するEntityManager
		/// @return 初期化済みのLua state
		std::unique_ptr<sol::state> CreateLuaState(EntityManager* entityManager);

	private:
		LuaRuntimeManager() = default;
		~LuaRuntimeManager() = default;
		LuaRuntimeManager(const LuaRuntimeManager&) = delete;
		LuaRuntimeManager& operator=(const LuaRuntimeManager&) = delete;

		/// @brief Lua stateに標準ライブラリを登録
		void RegisterStandardLibraries(sol::state* state);

		/// @brief Lua stateにQFE APIを登録
		void RegisterQFEAPI(sol::state* state, EntityManager* entityManager);
	};

}
