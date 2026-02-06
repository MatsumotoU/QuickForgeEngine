#include "engine/include/utility/script/ScriptUtility.h"
#include "engine/include/assets/AssetManager.h"

void QFE::Script::CompileScripts() {
	AssetManager* assetManager = AssetManager::GetInstance();
	std::string scriptsDir = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	if (!scriptsDir.empty() && scriptsDir.back() != '/' && scriptsDir.back() != '\\') {
		scriptsDir += '/';
	}
	std::string batPath = scriptsDir + "build_scripts.bat";
	std::string cmd = "call \"" + batPath + "\"";
	system(cmd.c_str());
}
