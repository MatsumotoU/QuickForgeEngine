#include "SetQFELinkers.h"
#include "LuaScriptOnQFESetGetterBase.h"
#include "LuaScriptOnQFESetStructBase.h"
#include "LuaScriptOnQFESetSubModuleBase.h"
#include "LuaScriptOnQFESetSceneFunction.h"
#include "LuaScriptOnQFESetUtilities.h"
#include "LuaScriptOnQEFSetMyMath.h"

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
