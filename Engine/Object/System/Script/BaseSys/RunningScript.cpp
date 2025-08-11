#include "RunningScript.h"
#include "Base/EngineCore.h"
#include <cassert>

void RunningScript::Run(EngineCore* engineCore, LuaScriptComponent* scriptComponent, const std::string& runFuncName) {
	if (!engineCore->GetLuaScriptResourceManager()->HasScript(scriptComponent->scriptName)) {
		assert(false && "Script not found in LuaScriptResourceManager.");
		return;
	}
	// Luaスクリプトリソースマネージャからスクリプトを取得
	auto luaState = engineCore->GetLuaScriptResourceManager()->GetScript(scriptComponent->scriptName);
	sol::function runFunction = (*luaState)[runFuncName];
	// スクリプトの実行関数が存在しない場合はエラーを出力
	if (!runFunction.valid()) {
		assert(false && "Run function is not valid.");
		return;
	}
	// スクリプトの実行関数を呼び出す
	try {
		runFunction(scriptComponent);
	}
	catch (const sol::error& e) {
		assert(false && ("Lua script error: " + std::string(e.what())).c_str());
	}
}
