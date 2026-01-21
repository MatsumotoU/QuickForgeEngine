#include "engine/include/assets/Script/CsharpVirtualEnvironmentOnQFE.h"
#include "engine/include/assets/Script/CsharpCompiler.h"

#include <windows.h>
#include <mono/metadata/metadata.h>
#include <mono/metadata/image.h>

#include <locale>
#include <codecvt>

#include "Engine/include/utility/String/MyString.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/assets/Script/QFElinker/CsharpOnQFELinker.h"
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF

void CsharpVirtualEnvironmentOnQFE::Initialize() {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Initializing Mono JIT...");
#endif // QFE_OPTIMIZE_OFF

	std::filesystem::path exeDir(path);
	exeDir = exeDir.parent_path();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Executable Directory: " + exeDir.string());
#endif // QFE_OPTIMIZE_OFF

	std::filesystem::path monoLibPath = exeDir / "mono" / "lib";
	std::filesystem::path monoEtcPath = exeDir / "mono" / "etc";

	// UTF-8変換
	std::string monoLibPathUtf8 = ConvertString(monoLibPath.wstring());
	std::string monoEtcPathUtf8 = ConvertString(monoEtcPath.wstring());

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Mono Lib Path: " + monoLibPath.string());
	DebugLog("Mono Etc Path: " + monoEtcPath.string());
#endif // QFE_OPTIMIZE_OFF

	try
	{
		mono_set_dirs(monoLibPathUtf8.c_str(), monoEtcPathUtf8.c_str());
	}
	catch (const std::exception& e)
	{
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::string("Failed to set Mono directories: ") + e.what());
#endif // QFE_OPTIMIZE_OFF

	}

#ifdef QFE_OPTIMIZE_OFF
	const char* options[] = {
		"--debugger-agent=transport=dt_socket,server=y,address=0.0.0.0:55555,suspend=n"
	};
	mono_jit_parse_options(sizeof(options) / sizeof(char*), (char**)options);

	DebugLog("Mono JIT options set for debugging.");
#endif // QFE_OPTIMIZE_OFF

	try
	{
		root_domain_ = mono_jit_init("QuickForgeRootDomain");
	}
	catch (const std::exception& e)
	{
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::string("Failed to initialize Mono JIT: ") + e.what());
#endif // QFE_OPTIMIZE_OFF

	}
	
	if (!root_domain_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to initialize Mono JIT.");
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	else {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Succsess to initialize Mono JIT.");
#endif // QFE_OPTIMIZE_OFF
	}
	ReloadAssembly();
}

void CsharpVirtualEnvironmentOnQFE::ResetScripts() {
	scripts_.clear();
}

void CsharpVirtualEnvironmentOnQFE::CreateCSProject(std::string& projectName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	std::string scriptsDir = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	std::string projectDir = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Project");
	std::string csprojPath = scriptsDir + projectName + ".csproj";
	std::string outputDllPath = projectDir + projectName + ".dll";
	GenerateCsproj(scriptsDir, csprojPath);
}

void CsharpVirtualEnvironmentOnQFE::CompileScripts() {
	AssetManager* assetManager = AssetManager::GetInstance();
	std::string scriptsDir = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	if (!scriptsDir.empty() && scriptsDir.back() != '/' && scriptsDir.back() != '\\') {
		scriptsDir += '/';
	}
	std::string batPath = scriptsDir + "build_scripts.bat";
	std::string cmd = "call \"" + batPath + "\"";
	system(cmd.c_str());
}

void CsharpVirtualEnvironmentOnQFE::OpenCSharpProjectInVSCode() {
	std::string scriptDir = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	std::string command = "code \"" + scriptDir + "\"";
	system(command.c_str());
}

void CsharpVirtualEnvironmentOnQFE::LinkQFEAPIToMono() {
	// Debug逕ｨAPI縺ｮ逋ｻ骭ｲ
	mono_add_internal_call("QuickForgeEngine.Debug::Log", (const void*)CsharpOnQFELinker::Native_Debug_Log);

	// Time謫堺ｽ懃畑API縺ｮ逋ｻ骭ｲ
	mono_add_internal_call("QuickForgeEngine.Time::GetDeltaTime", (const void*)CsharpOnQFELinker::GetDeltaTime);

	// Input謫堺ｽ懃畑API縺ｮ逋ｻ骭ｲ
	mono_add_internal_call("QuickForgeEngine.Input::GetKeyTrigger", (const void*)CsharpOnQFELinker::IsKeyTrigger);
	mono_add_internal_call("QuickForgeEngine.Input::GetKeyPress", (const void*)CsharpOnQFELinker::IsKeyPress);
	mono_add_internal_call("QuickForgeEngine.Input::GetKeyRelease", (const void*)CsharpOnQFELinker::IsKeyRelease);

	// Entity謫堺ｽ懃畑API縺ｮ逋ｻ骭ｲ
	mono_add_internal_call("QuickForgeEngine.Entity::Create", (const void*)CsharpOnQFELinker::CreateEntity);

	// Transform謫堺ｽ懃畑API縺ｮ逋ｻ骭ｲ
	mono_add_internal_call("QuickForgeEngine.TransformInternal::GetTranslate", (const void*)CsharpOnQFELinker::GetTransformRotate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::SetTranslate", (const void*)CsharpOnQFELinker::SetTransformTranslate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::GetRotate", (const void*)CsharpOnQFELinker::GetTransformRotate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::SetRotate", (const void*)CsharpOnQFELinker::SetTransformRotate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::GetScale", (const void*)CsharpOnQFELinker::GetTransformScale);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::SetScale", (const void*)CsharpOnQFELinker::SetTransformScale);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::Translate", (const void*)CsharpOnQFELinker::Translate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::Rotate", (const void*)CsharpOnQFELinker::Rotate);
}

void CsharpVirtualEnvironmentOnQFE::LoadAssembly() {
	if (!domain_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Mono domain not initialized.");
#endif // QFE_OPTIMIZE_OFF
		return;
	}

	if (assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Assembly already loaded.");
#endif // QFE_OPTIMIZE_OFF
		return;
	}

    // 螳溯｡後ヵ繧｡繧､繝ｫ縺ｮ繝代せ繧貞叙蠕・
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    // 螳溯｡後ヵ繧｡繧､繝ｫ縺ｮ繝・ぅ繝ｬ繧ｯ繝医Μ繧貞叙蠕・
    std::filesystem::path exeDir = std::filesystem::path(path).parent_path();

    // 隱ｭ縺ｿ霎ｼ繧DLL縺ｮ繝輔Ν繝代せ繧呈ｧ狗ｯ・(exe縺ｨ蜷後§髫主ｱ､)
    std::filesystem::path dllPath = exeDir / "CSharpScripts.dll";

	assembly_ = mono_domain_assembly_open(domain_, dllPath.string().c_str());
	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to load assembly: " + dllPath.string());
#endif // QFE_OPTIMIZE_OFF
	}
	return;
}

std::vector<std::string> CsharpVirtualEnvironmentOnQFE::GetAvailableScriptClasses() const {

	std::vector<std::string> classNames;

	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Error: Assembly is not loaded. Cannot get script class names. Check if 'CSharpScripts.dll' exists and is compiled correctly.");
#endif // QFE_OPTIMIZE_OFF
		return classNames;
	}

	MonoImage* image = mono_assembly_get_image(assembly_);

	if (!image) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Error: Could not get image from assembly.");
#endif // QFE_OPTIMIZE_OFF
		return classNames;
	}

	const MonoTableInfo* type_definitions_table = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

	if (!type_definitions_table) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Error: Could not get type definitions table from image.");
#endif // QFE_OPTIMIZE_OFF
		return classNames;
	}

	int num_types = mono_table_info_get_rows(type_definitions_table);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Scanning assembly for classes... Found {} type definitions.", num_types));
#endif // QFE_OPTIMIZE_OFF

	for (int i = 0; i < num_types; i++) {

		uint32_t cols[MONO_TYPEDEF_SIZE];

		mono_metadata_decode_row(type_definitions_table, i, cols, MONO_TYPEDEF_SIZE);

		const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		const char* ns = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);

		if (!name || name[0] == '<' || strstr(name, "_AnonStorey")) {
			continue;
		}

		std::string full_name;

		if (ns && strlen(ns) > 0) {
			full_name = std::string(ns) + "." + name;

		} else {
			full_name = name;
		}

		if (full_name.find("QuickForgeEngine") != std::string::npos) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::format("Found QuickForgeEngine class: {}", full_name));
#endif // QFE_OPTIMIZE_OFF
			continue;
		}
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::format("Found valid class: {}", full_name));
#endif // QFE_OPTIMIZE_OFF
		classNames.push_back(full_name);
	}

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Finished scanning. Returning {} valid classes.", classNames.size()));
#endif // QFE_OPTIMIZE_OFF
	return classNames;

}

uint32_t CsharpVirtualEnvironmentOnQFE::CreateScriptInstance(const std::string& className) {
	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Assembly not loaded. Cannot create script instance.");
#endif // QFE_OPTIMIZE_OFF
		return 0;
	}
	MonoImage* image = mono_assembly_get_image(assembly_);

	std::string ns_name;
	std::string class_name = className;
	size_t pos = className.rfind('.');
	if (pos != std::string::npos) {
		ns_name = className.substr(0, pos);
		class_name = className.substr(pos + 1);
	}

	MonoClass* monoClass = mono_class_from_name(image, ns_name.c_str(), class_name.c_str());
	if (!monoClass) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Class not found: " + className);
#endif // QFE_OPTIMIZE_OFF
		return 0;
	}
	MonoObject* instance = mono_object_new(domain_, monoClass);
	mono_runtime_object_init(instance);
	scripts_.push_back(instance);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Create Instance index: {}", static_cast<uint32_t>(scripts_.size()) - 1));
#endif // QFE_OPTIMIZE_OFF

	return static_cast<uint32_t>(scripts_.size()) - 1;
}

uint32_t CsharpVirtualEnvironmentOnQFE::CreateScriptInstance(uint32_t entityId, const std::string& className) {
	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Assembly not loaded. Cannot create script instance.");
		assert(false && "Assembly not loaded. Cannot create script instance.");
#endif // QFE_OPTIMIZE_OFF
		return 0;
	}
	MonoImage* image = mono_assembly_get_image(assembly_);

	std::string ns_name;
	std::string class_name = className;
	size_t pos = className.rfind('.');
	if (pos != std::string::npos) {
		ns_name = className.substr(0, pos);
		class_name = className.substr(pos + 1);
	}

	MonoClass* monoClass = mono_class_from_name(image, ns_name.c_str(), class_name.c_str());
	if (!monoClass) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Class not found: " + className);
#endif // QFE_OPTIMIZE_OFF
		return 0;
	}
	MonoObject* instance = mono_object_new(domain_, monoClass);
	mono_runtime_object_init(instance);
	MonoProperty* entityIdProperty = mono_class_get_property_from_name(monoClass, "EntityID");
	if (!entityIdProperty) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Property 'EntityID' not found in class: " + className);
#endif // QFE_OPTIMIZE_OFF
		return 0;
	}
	void* args[1];
	args[0] = &entityId;
	MonoObject* exception = nullptr;
	MonoMethod* setMethod = mono_property_get_set_method(entityIdProperty);
	mono_runtime_invoke(setMethod, instance, args, &exception);
	if (exception) {
		MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
		if (exceptionMsg) {
			char* exceptionCStr = mono_string_to_utf8(exceptionMsg);
#ifdef QFE_OPTIMIZE_OFF
			DebugLog(std::string("Mono Exception: ") + exceptionCStr);
#endif
			mono_free(exceptionCStr);
		}
	}

	scripts_.push_back(instance);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Create Instance index: {}", static_cast<uint32_t>(scripts_.size()) - 1));
#endif // QFE_OPTIMIZE_OFF

	return static_cast<uint32_t>(scripts_.size()) - 1;
}

void CsharpVirtualEnvironmentOnQFE::RunScriptFunction(uint32_t index, const std::string& functionName) {
	MonoObject* scriptInstance = scripts_.at(index);
	MonoClass* monoClass = mono_object_get_class(scriptInstance);
	MonoMethod* method = mono_class_get_method_from_name(monoClass, functionName.c_str(), 0);
	if (!method) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Method not found: " + functionName);
#endif // QFE_OPTIMIZE_OFF
		return;
	}
	MonoObject* exception = nullptr;
	mono_runtime_invoke(method, scriptInstance, nullptr, &exception);
	if (exception) {
		MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
		if (exceptionMsg) {
			char* exceptionCStr = mono_string_to_utf8(exceptionMsg);
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("C# Script[" + std::to_string(index) + "]:" + exceptionCStr, LogLevel::Error);
#endif
			mono_free(exceptionCStr);
		}
	}
}

void CsharpVirtualEnvironmentOnQFE::ReloadAssembly() {
	// 譌｢蟄倥・繧ｹ繧ｯ繝ｪ繝励ヨ繧､繝ｳ繧ｹ繧ｿ繝ｳ繧ｹ縺ｸ縺ｮ蜿ら・繧偵け繝ｪ繧｢
	scripts_.clear();
	assembly_ = nullptr;

	// 譌｢蟄倥・繧ｹ繧ｯ繝ｪ繝励ヨ繝峨Γ繧､繝ｳ繧偵い繝ｳ繝ｭ繝ｼ繝・
	if (domain_) {
		if (domain_ != root_domain_) {
			mono_domain_set(root_domain_, false);
			mono_domain_unload(domain_);
		}
		domain_ = nullptr;
	}

	// --- 譁ｰ縺励＞繝峨Γ繧､繝ｳ繧剃ｽ懈・ ---
	char domain_name[] = "QuickForgeScriptDomain";
	domain_ = mono_domain_create_appdomain(domain_name, nullptr);
	if (!domain_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to create new app domain.");
#endif
		return;
	}

	if (!mono_domain_set(domain_, false)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to set app domain.");
#endif
		return;
	}

	// --- C#繧ｹ繧ｯ繝ｪ繝励ヨ繧偵さ繝ｳ繝代う繝ｫ ---
#ifdef QFE_OPTIMIZE_OFF
	CompileScripts();
#endif // QFE_OPTIMIZE_OFF

	// --- 繧ｳ繝ｳ繝代う繝ｫ貂医∩謌先棡迚ｩ・・LL/PDB・峨ｒ繧ｨ繝ｳ繧ｸ繝ｳ縺ｮ螳溯｡後ヵ繧｡繧､繝ｫ繝・ぅ繝ｬ繧ｯ繝医Μ縺ｫ繧ｳ繝斐・ ---
	try {
		// 1. 繧ｳ繝斐・蜈・・繝代せ繧貞ｮ夂ｾｩ: C#繝励Ο繧ｸ繧ｧ繧ｯ繝医・繝・ヵ繧ｩ繝ｫ繝医ン繝ｫ繝牙・蜉帛・
		std::string scriptsBuildDir = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + "bin/Debug/netstandard2.0/";
		std::filesystem::path srcDllPath = scriptsBuildDir + "CSharpScripts.dll";
		std::filesystem::path srcPdbPath = scriptsBuildDir + "CSharpScripts.pdb";

		// 2. 繧ｳ繝斐・蜈医・繝代せ繧貞ｮ夂ｾｩ: 繧ｨ繝ｳ繧ｸ繝ｳ縺ｮ螳溯｡後ヵ繧｡繧､繝ｫ縺後≠繧九ョ繧｣繝ｬ繧ｯ繝医Μ
		wchar_t exePath[MAX_PATH];
		GetModuleFileNameW(NULL, exePath, MAX_PATH);
		std::filesystem::path destDir = std::filesystem::path(exePath).parent_path();

		// 3. 繝輔ぃ繧､繝ｫ繧偵さ繝斐・・域里蟄倥・繝輔ぃ繧､繝ｫ繧剃ｸ頑嶌縺搾ｼ・
		std::filesystem::copy(srcDllPath, destDir, std::filesystem::copy_options::overwrite_existing);
		std::filesystem::copy(srcPdbPath, destDir, std::filesystem::copy_options::overwrite_existing);

#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Copied C# artifacts to executable directory.");
#endif
	}
	catch (const std::filesystem::filesystem_error& e) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::string("Failed to copy C# artifacts: ") + e.what());
#endif
		// 繧ｳ繝斐・縺ｫ螟ｱ謨励＠縺溷ｴ蜷医・縲∝ｾ檎ｶ壹・DLL繝ｭ繝ｼ繝峨ｒ縺励↑縺・ｈ縺・掠譛溘Μ繧ｿ繝ｼ繝ｳ
		return;
	}
	// --- 繧ｳ繝斐・蜃ｦ逅・％縺薙∪縺ｧ ---

	// API繧貞・蠎ｦ繝ｪ繝ｳ繧ｯ
	LinkQFEAPIToMono();

	// 譁ｰ縺励＞繝峨Γ繧､繝ｳ縺ｫ繧｢繧ｻ繝ｳ繝悶Μ繧偵Ο繝ｼ繝・
	LoadAssembly();
}

void CsharpVirtualEnvironmentOnQFE::RunAllScriptsFunction(const std::string& functionName) {
	for (size_t i = 0; i < scripts_.size(); ++i) {
		RunScriptFunction(static_cast<uint32_t>(i), functionName);
	}
}

void CsharpVirtualEnvironmentOnQFE::Finalize() {
	// 繧ｹ繧ｯ繝ｪ繝励ヨ繝峨Γ繧､繝ｳ繧偵い繝ｳ繝ｭ繝ｼ繝・
	if (domain_ && domain_ != root_domain_) {
		// 繧ｫ繝ｬ繝ｳ繝医ラ繝｡繧､繝ｳ繧偵Ν繝ｼ繝医↓謌ｻ縺励※縺九ｉ繧｢繝ｳ繝ｭ繝ｼ繝峨☆繧九・縺悟ｮ牙・
		mono_domain_set(root_domain_, false);
		mono_domain_unload(domain_);
		domain_ = nullptr;
	}

	// Mono繝ｩ繝ｳ繧ｿ繧､繝蜈ｨ菴薙ｒ繧ｯ繝ｪ繝ｼ繝ｳ繧｢繝・・
	if (root_domain_) {
		mono_jit_cleanup(root_domain_);
		root_domain_ = nullptr;
	}
}
