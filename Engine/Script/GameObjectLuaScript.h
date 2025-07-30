#pragma once
#include "ILuaScript.h"

class BaseGameObject;
class EngineCore;

class GameObjectLuaScript : public ILuaScript {
public:
	GameObjectLuaScript(BaseGameObject* obj,EngineCore* engineCore);
	~GameObjectLuaScript() override = default;
	// LuaScript読み込み
	void LoadScript(const std::string& scriptFilePath);

public:
	void Init();
	void Update();
};