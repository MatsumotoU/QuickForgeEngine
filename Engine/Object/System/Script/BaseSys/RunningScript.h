#pragma once
#include "Object/Component/Data/LuaScriptComponent.h"
class EngineCore;

class RunningScript final {
public:
	/// <summary>
	/// 指定のLuaスクリプトの指定の引数なし関数を実行します。
	/// </summary>
	/// <param name="engineCore"></param>
	/// <param name="scriptComponent"></param>
	/// <param name="runFuncName"></param>
	static void Run(EngineCore* engineCore, LuaScriptComponent* scriptComponent,const std::string& runFuncName);
};