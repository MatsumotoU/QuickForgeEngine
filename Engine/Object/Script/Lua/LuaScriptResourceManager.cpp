#include "LuaScriptResourceManager.h"
#include "Base/EngineCore.h"
#include "Utility/CheckLoadFileExtension.h"

LuaScriptResourceManager::LuaScriptResourceManager(EngineCore* engineCore) :engineCore_(engineCore){}

void LuaScriptResourceManager::LoadScript(const std::string& key, const std::string& scriptName) {
	// スクリプトをロードする前に、既存のスクリプトを削除
	auto luaState = std::make_shared<sol::state>();
	luaState->open_libraries(sol::lib::base, sol::lib::package);

	// スクリプトファイルを読み込む
	std::string scriptFilePath = "Resources/Scripts/" + scriptName;
	CheckLoadFileExtension::Check(scriptFilePath, ".lua");
	sol::load_result loadResult = luaState->load_file(scriptFilePath);
	if (!loadResult.valid()) {
		throw std::runtime_error("Failed to load Lua script: " + scriptName);
	}

	// ここでスクリプトを実行する
	sol::protected_function_result execResult = loadResult();
	if (!execResult.valid()) {
		sol::error err = execResult;
		throw std::runtime_error("Failed to execute Lua script: " + std::string(err.what()));
	}

	luaScripts_[key] = luaState;
}

std::shared_ptr<sol::state> LuaScriptResourceManager::GetScript(const std::string& key) {
	return luaScripts_[key];
}

void LuaScriptResourceManager::RemoveScript(const std::string& key) {
	auto it = luaScripts_.find(key);
	if (it != luaScripts_.end()) {
		luaScripts_.erase(it);
	}
}

bool LuaScriptResourceManager::HasScript(const std::string& key) const {
	auto it = luaScripts_.find(key);
	if (it != luaScripts_.end()) {
		return true;
	}
	return false;
}
