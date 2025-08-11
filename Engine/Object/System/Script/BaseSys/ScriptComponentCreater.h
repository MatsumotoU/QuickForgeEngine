#pragma once
#include <string>
class EngineCore;
struct LuaScriptComponent;

class ScriptComponentCreater final{
public:
	static void Create(EngineCore* engineCore, LuaScriptComponent& scriptComponent, const std::string& scriptName);
};