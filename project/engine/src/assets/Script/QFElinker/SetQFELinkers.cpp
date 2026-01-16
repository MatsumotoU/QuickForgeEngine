#include "engine/include/assets/Script/QFElinker/SetQFELinkers.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetGetterBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetStructBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSubModuleBase.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetSceneFunction.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetUtilities.h"
#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetMyMath.h"

void QFE::Script::SetQFEFunctions(sol::state* luaState) {
	// 蝙九ｒ逋ｻ骭ｲ
	QFE::Script::Base::SetOnQFESetStructBase(luaState);
	// 螟画焚蜿門ｾ鈴未謨ｰ繧堤匳骭ｲ
	QFE::Script::Base::LuaScriptOnQFESetGetterBase(luaState);
	// 繧ｵ繝悶Δ繧ｸ繝･繝ｼ繝ｫ髢｢謨ｰ繧堤匳骭ｲ
	QFE::Script::Base::LuaScriptOnQFESetSubModuleBase(luaState);
	// 繧ｷ繝ｼ繝ｳ謫堺ｽ憺未謨ｰ繧堤匳骭ｲ
	QFE::Script::Scene::LuaScriptOnQFESetSceneFunction(luaState);
	// 繝ｦ繝ｼ繝・ぅ繝ｪ繝・ぅ髢｢謨ｰ繧堤匳骭ｲ
	QFE::Script::MyLuaMath::LuaScriptOnQFESetMyMath(luaState);
	QFE::Script::Utility::LuaScriptOnQFESetUtility(luaState);
}
