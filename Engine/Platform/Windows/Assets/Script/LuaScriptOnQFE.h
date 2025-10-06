#pragma once
#include <sol/sol.hpp>
#include <string>
#include <memory>
#include <map>
#include <vector>
#include <stdint.h>
#include "Core/Math/Vector/Vector3.h"

#include <set>

class LuaScriptOnQFE {
public:
	LuaScriptOnQFE();
	~LuaScriptOnQFE() = default;
	void LoadScript(const std::string& scriptName);
	void RunFunction(const std::string& functionName);
	bool HasFunction(const std::string& functionName) const;
	std::vector<std::string> GetFunctionList() const;
	sol::state* GetScript() const;
	const bool& IsCanRun() const;
	bool IsAliveEntity();

	void SetEntityValue(uint32_t entityId);
	uint32_t GetBindEntityId() const { return bindEntityId_; }
	std::vector<std::string> GetGlobalValuesList() const;
	std::set<std::string>& GetGlobals() { return UserGlobals; }
	std::string GetScriptName() const { return scriptName_; }

private:
	/// QFE用の関数を登録
	void SetQFEFunctions();

	// QFE用の関数
	void SetPosition(uint32_t entityId,const Vector3& position);

	bool isCanRun_;
	uint32_t bindEntityId_;
	std::string scriptName_;
	std::set<std::string> defaultGlobals;
	std::set<std::string> UserGlobals;
	std::unique_ptr<sol::state> luaState_;
};