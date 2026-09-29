#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetGetterBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetStructBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSubModuleBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSceneFunction.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetUtilities.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQEFSetMyMath.h"

void QFE::Script::SetQFEFunctions(sol::state* luaState) {
	// 型を登録
	QFE::Script::Base::SetOnQFESetStructBase(luaState);
	// 変数取得関数を登録
	QFE::Script::Base::LuaScriptOnQFESetGetterBase(luaState);
	// サブモジュール関数を登録
	QFE::Script::Base::LuaScriptOnQFESetSubModuleBase(luaState);
	// シーン操作関数を登録
	QFE::Script::Scene::LuaScriptOnQFESetSceneFunction(luaState);
	// ユーティリティ関数を登録
	QFE::Script::MyLuaMath::LuaScriptOnQEFSetMyMath(luaState);
	QFE::Script::Utility::LuaScriptOnQFESetUtility(luaState);
}
