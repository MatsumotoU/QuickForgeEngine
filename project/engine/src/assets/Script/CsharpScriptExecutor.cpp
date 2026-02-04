#include "engine/include/assets/Script/CsharpScriptExecutor.h"
#include "engine/include/assets/Script/MonoRuntimeManager.h"
#include "engine/include/core/Entity/EntityManager.h"

#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif

#include <format>

using namespace QFE;

CsharpScriptExecutor::CsharpScriptExecutor() {}

CsharpScriptExecutor::~CsharpScriptExecutor() {
	Finalize();
}

void CsharpScriptExecutor::Initialize(EntityManager* entityManager) {
	entityManager_ = entityManager;

	// MonoRuntimeManagerが初期化されているか確認
	if (!MonoRuntimeManager::GetInstance()->IsInitialized()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("MonoRuntimeManager is not initialized.", LogLevel::Error);
#endif
		return;
	}

	// このシーン専用のAppDomainを作成
	MonoDomain* rootDomain = MonoRuntimeManager::GetInstance()->GetRootDomain();

	char domainName[] = "QuickForgeSceneDomain";
	domain_ = mono_domain_create_appdomain(domainName, nullptr);

	if (!domain_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to create AppDomain for scene.", LogLevel::Error);
#endif
		return;
	}

	// ドメインを切り替え
	if (!mono_domain_set(domain_, false)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to set AppDomain.", LogLevel::Error);
#endif
		return;
	}

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("CsharpScriptExecutor initialized with new AppDomain.");
#endif

	// アセンブリをロード
	LoadAssembly();
}

void CsharpScriptExecutor::LoadAssembly() {
	if (!domain_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Domain not initialized. Cannot load assembly.", LogLevel::Error);
#endif
		return;
	}

	std::string assemblyPath = MonoRuntimeManager::GetInstance()->GetAssemblyPath();
	assembly_ = mono_domain_assembly_open(domain_, assemblyPath.c_str());

	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to load assembly: " + assemblyPath, LogLevel::Error);
#endif
	} else {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Assembly loaded successfully: " + assemblyPath);
#endif
	}
}

std::vector<std::string> CsharpScriptExecutor::GetAvailableScriptClasses() const {
	std::vector<std::string> classNames;

	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Error: Assembly is not loaded. Cannot get script class names.", LogLevel::Error);
#endif
		return classNames;
	}

	MonoImage* image = mono_assembly_get_image(assembly_);

	if (!image) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Error: Could not get image from assembly.", LogLevel::Error);
#endif
		return classNames;
	}

	const MonoTableInfo* type_definitions_table = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

	if (!type_definitions_table) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Error: Could not get type definitions table from image.", LogLevel::Error);
#endif
		return classNames;
	}

	int num_types = mono_table_info_get_rows(type_definitions_table);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Scanning assembly for classes... Found {} type definitions.", num_types));
#endif

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
#endif
			continue;
		}
#ifdef QFE_OPTIMIZE_OFF
		DebugLog(std::format("Found valid class: {}", full_name));
#endif
		classNames.push_back(full_name);
	}

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Finished scanning. Returning {} valid classes.", classNames.size()));
#endif
	return classNames;
}

uint32_t CsharpScriptExecutor::CreateScriptInstance(uint32_t entityId, const std::string& className) {
	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Assembly not loaded. Cannot create script instance.", LogLevel::Error);
#endif
		return 0;
	}

	MonoImage* image = mono_assembly_get_image(assembly_);

	// クラス名をパース
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
		DebugLog("Class not found: " + className, LogLevel::Error);
#endif
		return 0;
	}

	MonoObject* instance = mono_object_new(domain_, monoClass);
	mono_runtime_object_init(instance);

	// EntityIDプロパティを設定
	MonoProperty* entityIdProperty = mono_class_get_property_from_name(monoClass, "EntityID");
	if (entityIdProperty) {
		void* args[1] = { &entityId };
		MonoObject* exception = nullptr;
		MonoMethod* setMethod = mono_property_get_set_method(entityIdProperty);
		mono_runtime_invoke(setMethod, instance, args, &exception);

		if (exception) {
			MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
			if (exceptionMsg) {
				char* exceptionCStr = mono_string_to_utf8(exceptionMsg);
#ifdef QFE_OPTIMIZE_OFF
				DebugLog(std::string("Mono Exception: ") + exceptionCStr, LogLevel::Error);
#endif
				mono_free(exceptionCStr);
			}
		}
	}

	scripts_.push_back(instance);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Created C# script instance: {} (index: {})", className, scripts_.size() - 1));
#endif

	return static_cast<uint32_t>(scripts_.size()) - 1;
}

void QFE::CsharpScriptExecutor::CreateScriptInstance(const std::string& className) {
	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Assembly not loaded. Cannot create script instance.");
#endif // QFE_OPTIMIZE_OFF
		return;
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
		return;
	}
	MonoObject* instance = mono_object_new(domain_, monoClass);
	mono_runtime_object_init(instance);
	scripts_.push_back(instance);

#ifdef QFE_OPTIMIZE_OFF
	DebugLog(std::format("Create Instance index: {}", static_cast<uint32_t>(scripts_.size()) - 1));
#endif // QFE_OPTIMIZE_OFF
	return;
}

void CsharpScriptExecutor::DeleteScriptInstance(uint32_t index) {
	if (index >= scripts_.size()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Invalid script index: " + std::to_string(index), LogLevel::Error);
#endif
		return;
	}
	scripts_.erase(scripts_.begin() + index);
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("Deleted script instance at index: " + std::to_string(index));
#endif
}

void CsharpScriptExecutor::RunScriptFunction(uint32_t index, const std::string& functionName) {
	if (index >= scripts_.size()) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Invalid script index: " + std::to_string(index), LogLevel::Error);
#endif
		return;
	}

	MonoObject* scriptInstance = scripts_.at(index);
	MonoClass* monoClass = mono_object_get_class(scriptInstance);
	MonoMethod* method = mono_class_get_method_from_name(monoClass, functionName.c_str(), 0);

	if (!method) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Method not found: " + functionName, LogLevel::Warning);
#endif
		return;
	}

	MonoObject* exception = nullptr;
	mono_runtime_invoke(method, scriptInstance, nullptr, &exception);

	if (exception) {
		MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
		if (exceptionMsg) {
			char* exceptionCStr = mono_string_to_utf8(exceptionMsg);
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("C# Script[" + std::to_string(index) + "]: " + exceptionCStr, LogLevel::Error);
#endif
			mono_free(exceptionCStr);
		}
	}
}

void CsharpScriptExecutor::RunAllScriptsFunction(const std::string& functionName) {
	for (size_t i = 0; i < scripts_.size(); ++i) {
		RunScriptFunction(static_cast<uint32_t>(i), functionName);
	}
}

void CsharpScriptExecutor::ResetScripts() {
	scripts_.clear();
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("All C# scripts reset.");
#endif
}

void CsharpScriptExecutor::ReloadAssembly() {
	// スクリプトインスタンスをクリア
	scripts_.clear();
	assembly_ = nullptr;

	// 既存のドメインをアンロード
	if (domain_) {
		MonoDomain* rootDomain = MonoRuntimeManager::GetInstance()->GetRootDomain();
		if (domain_ != rootDomain) {
			mono_domain_set(rootDomain, false);
			mono_domain_unload(domain_);
		}
		domain_ = nullptr;
	}

	// 新しいドメインを作成
	char domainName[] = "QuickForgeSceneDomain";
	domain_ = mono_domain_create_appdomain(domainName, nullptr);
	if (!domain_) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to create new app domain.", LogLevel::Error);
#endif
		return;
	}

	if (!mono_domain_set(domain_, false)) {
#ifdef QFE_OPTIMIZE_OFF
		DebugLog("Failed to set app domain.", LogLevel::Error);
#endif
		return;
	}

	// アセンブリをリロード
	LoadAssembly();

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("C# assembly reloaded.");
#endif
}

void CsharpScriptExecutor::Finalize() {
	scripts_.clear();
	assembly_ = nullptr;

	if (domain_) {
		MonoDomain* rootDomain = MonoRuntimeManager::GetInstance()->GetRootDomain();
		if (domain_ != rootDomain) {
			mono_domain_set(rootDomain, false);
			mono_domain_unload(domain_);
		}
		domain_ = nullptr;
	}

#ifdef QFE_OPTIMIZE_OFF
	DebugLog("CsharpScriptExecutor finalized.");
#endif
}
