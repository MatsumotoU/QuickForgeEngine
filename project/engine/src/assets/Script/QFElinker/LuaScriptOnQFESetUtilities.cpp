#include "LuaScriptOnQFESetUtilities.h"
#include "engine/include/assets/AssetManager.h"
#ifdef _DEBUG
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

#include "engine/include/utility/FileSystems/FileUtility.h"

void QFE::Script::Utility::LuaScriptOnQFESetUtility(sol::state* luaState) {
	

	// CSV読み込み
	luaState->set_function("Load2DMap", [](const std::string& fileName) {
		std::vector<std::vector<uint32_t>> result;
		std::string path = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("2DMap") + fileName;
#ifdef _DEBUG
		DebugLog("Load2DMap: " + path);
#endif // _DEBUG

		if (QFE::FILE::LoadCSVToVector(path, result)) {
			return sol::as_table(result);
		} else {
#ifdef _DEBUG
			DebugLog("Failed to load 2D map: " + path,LogLevel::Error);
#endif // _DEBUG
			return sol::as_table(std::vector<std::vector<uint32_t>>{});
		}
		});

}
