#include "ScriptComponentCreater.h"
#include "Base/EngineCore.h"
#include "Object/Component/Data/LuaScriptComponent.h"

// TODO: テーブル名を改変
void ScriptComponentCreater::Create(EngineCore* engineCore, LuaScriptComponent& scriptComponent, const std::string& scriptName) {
	scriptComponent.scriptName = scriptName;
	// スクリプトの関数名を取得
	auto luaState = engineCore->GetLuaScriptResourceManager()->GetScript(scriptName);
	// スクリプトが存在しない場合はエラーを出力
	if (!luaState) {
		engineCore->GetLuaScriptResourceManager()->LoadScript(scriptName,scriptName);
		luaState = engineCore->GetLuaScriptResourceManager()->GetScript(scriptName);
	}

	// スクリプトの関数名と変数名を取得
	sol::table myTable = (*luaState)["MyScript"];
	if (myTable.valid()) {
		for (const auto& [key, value] : myTable) {
			if (value.get_type() == sol::type::function) {
				scriptComponent.funcNames.push_back(key.as<std::string>());
			} else if (value.get_type() == sol::type::userdata) {
				scriptComponent.valueNames.push_back(key.as<std::string>());
			}
		}
	}
}
