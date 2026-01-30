#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetGetterBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetStructBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSubModuleBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSceneFunction.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetUtilities.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetMyMath.h"
#include "engine/include/core/Entity/EntityManager.h"

void QFE::Script::SetQFEFunctions(sol::state* luaState, EntityManager* entityManager) {
	// 型を登録
	QFE::Script::Base::SetOnQFESetStructBase(luaState);
	// 変数取得関数を登録
	QFE::Script::Base::LuaScriptOnQFESetGetterBase(luaState, entityManager);
	// サブモジュール関数を登録
	QFE::Script::Base::LuaScriptOnQFESetSubModuleBase(luaState, entityManager);
	// シーン操作関数を登録
	QFE::Script::Scene::LuaScriptOnQFESetSceneFunction(luaState, entityManager);
	// ユーティリティ関数を登録
	QFE::Script::MyLuaMath::LuaScriptOnQFESetMyMath(luaState);
	QFE::Script::Utility::LuaScriptOnQFESetUtility(luaState, entityManager);
}
