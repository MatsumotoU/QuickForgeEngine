#pragma once
#include <sol/sol.hpp>
#include <memory>
#include <unordered_map>
#include "engine/include/assets/Script/LuaScriptOnQFE.h"

namespace QFE {

	class EntityManager;

	/**
	 * @brief シーン固有のLuaスクリプト実行環境
	 * 各シーンが独自のsol::stateとスクリプトインスタンスを持つ
	 */
	class LuaScriptExecutor {
	public:
		LuaScriptExecutor();
		~LuaScriptExecutor() = default;

		/// @brief 初期化（EntityManagerへの参照を設定し、Lua stateを作成）
		void Initialize(EntityManager* entityManager);

		/// @brief フレーム開始時の処理
		void FrameStart();

		/// @brief 全スクリプトをリセット
		void Reset();

		/// @brief 全スクリプトをリロード
		void ReloadAllScripts();

		/// @brief スクリプトを追加
		/// @param entityId バインドするエンティティID
		/// @param scriptName スクリプト名
		/// @return スクリプトハンドル
		uint32_t AddScript(uint32_t entityId, const std::string& scriptName);

		/// @brief スクリプトを削除
		/// @param handle スクリプトハンドル
		void RemoveScript(uint32_t handle);

		/// @brief 全スクリプトを初期化
		void InitializeAllScripts();

		/// @brief 全スクリプトを更新
		void UpdateAllScripts();

		/// @brief 全スクリプトの指定関数を実行
		/// @param functionName 関数名
		void RunAllFunction(const std::string& functionName);

		/// @brief あるエンティティの関数を実行
		/// @param entityId エンティティID
		/// @param functionName 関数名
		void RunEntityFunction(uint32_t entityId, const std::string& functionName);

		/// @brief あるエンティティの関数を実行（引数付き）
		template<typename... Args>
		void RunEntityFunctionWithArgs(uint32_t entityId, const std::string& functionName, Args&&... args) {
			for (auto& [handle, script] : scripts_) {
				if (script && script->GetBindEntityId() == entityId) {
					script->RunFunction(functionName, std::forward<Args>(args)...);
				}
			}
		}

		/// @brief 指定エンティティの指定スクリプトの関数を実行
		void RunFunction(uint32_t entityId, const std::string& scriptName, const std::string& functionName);
		
		/// @brief 指定エンティティのスクリプトからグローバル変数を取得
		sol::object GetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::state_view& callerState);

		/// @brief 指定エンティティのスクリプトにグローバル変数を設定
		void SetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName, const sol::object& value);

		/// @brief スクリプトを取得
		/// @param handle スクリプトハンドル
		/// @return スクリプトポインタ
		LuaScriptOnQFE* GetScript(uint32_t handle) const;

		/// @brief Lua stateを取得
		sol::state& GetSharedState() { return *sharedLuaState_; }

		/// @brief スクリプト数を取得
		int GetScriptCount() const { return static_cast<int>(scripts_.size()); }

		/// @brief スクリプト実行中フラグ
		bool isRunningScript_ = false;

	private:
		EntityManager* entityManager_ = nullptr;
		std::unique_ptr<sol::state> sharedLuaState_;  // このシーン専用のLua state
		std::unordered_map<uint32_t, std::unique_ptr<LuaScriptOnQFE>> scripts_;
		uint32_t nextScriptHandle_ = 0;
	};

}
