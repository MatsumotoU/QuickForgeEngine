#include "LuaScriptManager.h"
#pragma comment(lib, "Lua.lib")

#include "Object/BaseGameObject.h"
#include "Script/GameObjectLuaScript.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

LuaScriptManager::LuaScriptManager(EngineCore* engineCore) {
	luaScripts_.clear();
	engineCore_ = engineCore;
}

LuaScriptManager::~LuaScriptManager() {}

void LuaScriptManager::ClearAllGameObjScripts() {
	luaScripts_.clear();
}

void LuaScriptManager::AddGameObjScript(const std::string& key, BaseGameObject* obj, const std::string& scriptFilePath) {
	auto script = std::make_unique<GameObjectLuaScript>(obj,engineCore_);
	script->LoadScript(scriptFilePath);
	luaScripts_[key] = std::move(script);
}

GameObjectLuaScript* LuaScriptManager::GetScript(const std::string& key) {
	auto it = luaScripts_.find(key);
	if (it != luaScripts_.end()) {
		return dynamic_cast<GameObjectLuaScript*>(it->second.get());
	}
	return nullptr;
}

void LuaScriptManager::RemoveScript(const std::string& key) {
	luaScripts_.erase(key);
}

void LuaScriptManager::InitScripts() {
	for (auto& scriptPair : luaScripts_) {
		auto* script = dynamic_cast<GameObjectLuaScript*>(scriptPair.second.get());
		if (script) {
			script->Init();
		}
	}
}

void LuaScriptManager::UpdateScripts() {
	for (auto& scriptPair : luaScripts_) {
		auto* script = dynamic_cast<GameObjectLuaScript*>(scriptPair.second.get());
		if (script) {
			script->Update();
		}
	}
}
