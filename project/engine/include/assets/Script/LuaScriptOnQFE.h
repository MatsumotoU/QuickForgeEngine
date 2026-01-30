#pragma once
#include <sol/sol.hpp>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <stdint.h>

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF
#include "engine/include/core/Entity/EntityManager.h"

#include <set>

namespace QFE {

	class SceneObjectData;

	class LuaScriptOnQFE {
	public:
		LuaScriptOnQFE() = delete;
		explicit LuaScriptOnQFE(EntityManager* entityManager);
		~LuaScriptOnQFE() = default;

		// コピーコンストラクタとムーブコンストラクタを削除
		LuaScriptOnQFE(const LuaScriptOnQFE&) = delete;
		LuaScriptOnQFE(LuaScriptOnQFE&&) = delete;
		LuaScriptOnQFE& operator=(const LuaScriptOnQFE&) = delete;
		LuaScriptOnQFE& operator=(LuaScriptOnQFE&&) = delete;

		void Initialize(sol::state* state, const std::string& scriptPath, uint32_t bindId);

		void LoadScript(const std::string& scriptName);
		void ReloadScript();
		template<typename... Args>
		inline void RunFunction(const std::string& functionName, Args&&... args) {
#ifdef QFE_OPTIMIZE_OFF
			try {
				if (!isCanRun_) {
					throw std::runtime_error("Cannot run function. Lua script is not loaded or failed to load.");
				}
				sol::object obj = environment_[functionName];
				if (!obj.is<sol::function>()) {
					throw std::runtime_error("Function '" + functionName + "' not found in Lua script.");
				}
				sol::protected_function func = obj.as<sol::protected_function>();
				sol::protected_function_result result = func(std::forward<Args>(args)...);
				if (!result.valid()) {
					sol::error err = result;
					DebugLog("Lua error in '" + functionName + "': " + std::string(err.what()), LogLevel::Error);
				}
			}
			catch (const std::exception& e) {
				DebugLog("Error in Lua function '" + functionName + "': " + e.what(), LogLevel::Error);
			}
#else

			if (!isCanRun_) {
				return;
			}
			auto it = functionCache_.find(functionName);
			sol::function func;
			if (it != functionCache_.end()) {
				func = it->second;
			} else {
				sol::object obj = environment_[functionName];
				if (!obj.is<sol::function>()) {
					return;
				}
				func = obj.as<sol::function>();
				functionCache_[functionName] = func;
			}
			func(std::forward<Args>(args)...);
#endif
		}

		bool HasFunction(const std::string& functionName) const;
		void RunInit();
		void RunUpdate();
		void RunCollisionEnter(uint32_t id, SceneObjectData* objData);
		void RunCollisionStay(uint32_t id, SceneObjectData* objData);
		std::vector<std::string> GetFunctionList() const;
		sol::state* GetScript() const;
		sol::environment& GetEnvironment() { return environment_; }
		const bool& IsCanRun() const;

		bool IsAliveEntity();

		void SetEntityValue(uint32_t entityId);
		void SetPriority(uint32_t priority) { priority_ = priority; }
		uint32_t GetBindEntityId() const { return bindEntityId_; }
		std::vector<std::string> GetGlobalValuesList() const;
		std::set<std::string>& GetGlobals() { return UserGlobals; }
		std::string GetScriptName() const { return scriptName_; }
		uint32_t GetPriority() const { return priority_; }
		void SetHandle(uint32_t handle) { handle_ = handle; }
		uint32_t GetHandle() const { return handle_; }


	private:
		/// QFE逕ｨ縺ｮ髢｢謨ｰ繧堤匳骭ｲ
		void SetQFEFunctions();

		bool isCanRun_;
		uint32_t bindEntityId_;
		uint32_t handle_;
		uint32_t priority_;

		std::string scriptName_;
		std::set<std::string> defaultGlobals;
		std::set<std::string> UserGlobals;
		sol::environment environment_;


#ifndef QFE_OPTIMIZE_OFF
		std::map<std::string, sol::function> functionCache_;
#endif
		sol::function initFunc_;
		sol::function updateFunc_;
		sol::function onCollisionEnterFunc_;
		sol::function onCollisionStayFunc_;

		EntityManager* entityManager_;
		sol::state* luaState_;
	};

}
