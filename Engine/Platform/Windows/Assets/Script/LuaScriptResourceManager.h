#pragma once
#include <vector>
#include <memory>
#include <string>
#include "LuaScriptOnQFE.h"
#include "Utility/DesignPatterns/Singleton.h"

class LuaScriptResourceManager final :public Singleton<LuaScriptResourceManager>{
	friend class Singleton<LuaScriptResourceManager>;
	LuaScriptResourceManager() = default;
	LuaScriptResourceManager(const LuaScriptResourceManager&) = delete;
	LuaScriptResourceManager& operator=(const LuaScriptResourceManager&) = delete;
	LuaScriptResourceManager(LuaScriptResourceManager&&) = delete;
	LuaScriptResourceManager& operator=(LuaScriptResourceManager&&) = delete;
public:
	void Initialize();
	uint32_t AddScript(uint32_t entityId,const std::string& scriptName);
	void RemoveScript(uint32_t handle);
	void InitializeAllScripts();
	void UpdateAllScripts();
	void EndFrame();
	void Finalize();

private:
	void CheckScriptEntity();
	std::vector<uint32_t> removeScriptHandles_;
	std::vector<std::unique_ptr<LuaScriptOnQFE>> scripts_;
};