#include "engine/include/utility/script/ScriptUtility.h"
#include "engine/include/assets/AssetManager.h"
#include <fstream>

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

void QFE::Script::CreateScriptTemplate(const std::string& scriptName, const std::string& outputPath){
	try
	{
		std::string fullPath = outputPath;
		if (!fullPath.empty() && fullPath.back() != '/' && fullPath.back() != '\\') {
			fullPath += '/';
		}
		fullPath += scriptName + ".cs";

		std::ofstream file(fullPath);
		if (file.is_open()) {
			file << "using System;\n";
			file << "using QuickForgeEngine;\n\n";
			file << "public class " << scriptName << " : QuickForgeComponent\n";
			file << "{\n";
			file << "    public override void Initialize(){\n";
			file << "    }\n";
			file << "    public override void Update(){\n";
			file << "    }\n";
			file << "}\n";
			file.close();
		}
		QFE_LOG("Script template created successfully: " + fullPath);
	}
	catch (const std::exception& e)
	{
		QFE_LOG(std::string("Failed to create script template: ") + e.what());
	}
}
