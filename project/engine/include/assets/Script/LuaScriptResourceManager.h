#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include "LuaScriptOnQFE.h"
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "engine/include/scene/Data/SceneObjectData.h"

class LuaScriptResourceManager final :public Singleton<LuaScriptResourceManager> {
	friend class Singleton<LuaScriptResourceManager>;
	LuaScriptResourceManager() = default;
	LuaScriptResourceManager(const LuaScriptResourceManager&) = delete;
	LuaScriptResourceManager& operator=(const LuaScriptResourceManager&) = delete;
	LuaScriptResourceManager(LuaScriptResourceManager&&) = delete;
	LuaScriptResourceManager& operator=(LuaScriptResourceManager&&) = delete;
public:
	void Initialize();
	void Reset();
	void ReloadAllScripts();

	void RunAllFunction(const std::string& functionName);
	void CreateScript(const std::string& scriptName);
	uint32_t AddScript(uint32_t entityId, const std::string& scriptName);
	void RequestRemoveScript(uint32_t handle);
	void OpenAndEditScript(const std::string& scriptName);
	void RemoveScript(uint32_t handle);
	void InitializeAllScripts();
	void InitializeScript(uint32_t handle);
	void UpdateAllScripts();
	void RunColliderStay(uint32_t runId,uint32_t id, SceneObjectData* objData);
	void RunTriggerEnter(uint32_t runId,uint32_t id, SceneObjectData* objData);
	void EndFrame();
	void Finalize();

	LuaScriptOnQFE* GetScript(uint32_t handle) const;
	std::set<std::string>& GetScriptGlobals(uint32_t entityId) const;
	sol::object GetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName,sol::state_view callScriptState);
	void SetEntityScriptGlobal(uint32_t entityId, const std::string& scriptName, const std::string& varName, sol::object value);
	void RunFunction(uint32_t entityId, const std::string& scriptName, const std::string& functionName);

	bool isRunningScript_;
private:
	uint32_t maxPriority_;
	void CopyLuaTable(const sol::table& src, sol::table& dst);
	void CheckScriptEntity();
	std::vector<uint32_t> removeScriptHandles_;
	std::unordered_map<uint32_t, std::unique_ptr<LuaScriptOnQFE>> scripts_;
	uint32_t nextScriptHandle_;
};
