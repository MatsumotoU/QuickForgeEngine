#pragma once
#include <sol/sol.hpp>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <stdint.h>

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include <set>

class LuaScriptOnQFE {
public:
	LuaScriptOnQFE();
	~LuaScriptOnQFE() = default;
	void LoadScript(const std::string& scriptName);
	void ReloadScript();
	template<typename... Args>
	inline void RunFunction(const std::string& functionName, Args&&... args) {
		try {
			if (!isCanRun_) {
				throw std::runtime_error("Cannot run function. Lua script is not loaded or failed to load.");
			}
			if (!luaState_) {
				throw std::runtime_error("Lua state is not initialized.");
			}
			sol::object obj = luaState_->get<sol::object>(functionName);
			if (!obj.is<sol::function>()) {
				throw std::runtime_error("Function '" + functionName + "' not found in Lua script.");
			}
			sol::protected_function func = obj.as<sol::protected_function>();
			sol::protected_function_result result = func(std::forward<Args>(args)...);
			if (!result.valid()) {
				sol::error err = result;
#ifdef _DEBUG
				DebugLog("Lua error in '" + functionName + "': " + std::string(err.what()), LogLevel::Error);
#else
				std::cerr << "Lua error in '" << functionName << "': " << err.what() << std::endl;
#endif
			}
		}
		catch (const std::exception& e) {
#ifdef _DEBUG
			DebugLog("Error in Lua function '" + functionName + "': " + e.what(), LogLevel::Error);
#else
			std::cerr << "Error in Lua function '" << functionName << "': " << e.what() << std::endl;
#endif
		}
	}
	bool HasFunction(const std::string& functionName) const;
	std::vector<std::string> GetFunctionList() const;
	sol::state* GetScript() const;
	const bool& IsCanRun() const;
	bool IsAliveEntity();

	void SetEntityValue(uint32_t entityId);
	void SetPriority(uint32_t priority) { priority_ = priority; }
	uint32_t GetBindEntityId() const { return bindEntityId_; }
	std::vector<std::string> GetGlobalValuesList() const;
	std::set<std::string>& GetGlobals() { return UserGlobals; }
	std::string GetScriptName() const { return scriptName_; }
	uint32_t GetPriority() const { return priority_; }

private:
	/// QFE用の関数を登録
	void SetQFEFunctions();

	bool isCanRun_;
	uint32_t bindEntityId_;
	uint32_t priority_;
	std::string scriptName_;
	std::set<std::string> defaultGlobals;
	std::set<std::string> UserGlobals;
	std::unique_ptr<sol::state> luaState_;
};