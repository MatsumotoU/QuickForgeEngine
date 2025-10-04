#pragma once
#include <vector>
#include <memory>
#include <string>
#include "LuaScriptOnQFE.h"
#include "Utility/DesignPatterns/Singleton.h"

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
	void CreateScript(const std::string& scriptName);
	uint32_t AddScript(uint32_t entityId, const std::string& scriptName);
	void RequestRemoveScript(uint32_t handle);
	void OpenAndEditScript(const std::string& scriptName);
	void RemoveScript(uint32_t handle);
	void InitializeAllScripts();
	void UpdateAllScripts();
	void RunColliderStay(uint32_t aEintityId, uint32_t bEintityId);
	void EndFrame();
	void Finalize();

	LuaScriptOnQFE* GetScript(uint32_t handle) const;
	std::set<std::string>& GetScriptGlobals(uint32_t entityId) const;


private:
	void CheckScriptEntity();
	std::vector<uint32_t> removeScriptHandles_;
	std::vector<std::unique_ptr<LuaScriptOnQFE>> scripts_;
};