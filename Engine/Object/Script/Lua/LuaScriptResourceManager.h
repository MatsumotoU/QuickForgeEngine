#pragma once
#include <map>
#include <memory>
#include <string>
#include <sol/sol.hpp>

class EngineCore;

class LuaScriptResourceManager {
public:
	LuaScriptResourceManager(EngineCore* engineCore);
	~LuaScriptResourceManager() = default;
	/// <summary>
	/// スクリプト追加
	/// </summary>
	/// <param name="key"></param>
	/// <param name="scriptFilePath"></param>
	void LoadScript(const std::string& key, const std::string& scriptName);
	/// <summary>
	/// スクリプト取得
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	std::shared_ptr<sol::state> GetScript(const std::string& key);
	/// <summary>
	/// スクリプト削除
	/// </summary>
	/// <param name="key"></param>
	void RemoveScript(const std::string& key);
	/// <summary>
	/// スクリプトがあるかどうか
	/// </summary>
	/// <param name="key"></param>
	/// <returns></returns>
	bool HasScript(const std::string& key) const;

private:
	EngineCore* engineCore_;
	std::map<std::string, std::shared_ptr<sol::state>> luaScripts_; // スクリプトのキーとsol::stateのマップ
};