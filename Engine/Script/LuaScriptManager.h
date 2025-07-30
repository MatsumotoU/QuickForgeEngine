#pragma once
#include <map>
#include <memory>
#include "ILuaScript.h"

class BaseGameObject;
class GameObjectLuaScript;
class EngineCore;

class LuaScriptManager {
public:
    LuaScriptManager(EngineCore* engineCore);
    ~LuaScriptManager();

    void ClearAllGameObjScripts();
    /// <summary>
    /// スクリプト追加
    /// </summary>
    /// <param name="key"></param>
    /// <param name="obj"></param>
    /// <param name="scriptFilePath"></param>
    void AddGameObjScript(const std::string& key, BaseGameObject* obj, const std::string& scriptFilePath);
    /// <summary>
	/// スクリプト取得
    /// </summary>
    /// <param name="key"></param>
    /// <returns></returns>
    GameObjectLuaScript* GetScript(const std::string& key);
    /// <summary>
    /// スクリプト削除
    /// </summary>
    /// <param name="key"></param>
    void RemoveScript(const std::string& key);

public:
    void InitScripts();
	void UpdateScripts();

private:
	EngineCore* engineCore_;
	std::map<std::string, std::unique_ptr<ILuaScript>> luaScripts_;
};