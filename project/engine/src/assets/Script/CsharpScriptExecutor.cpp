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
		QFE_LOG("MonoRuntimeManager is not initialized.", LogLevel::Error);
#endif
		return;
	}

	// このシーン専用のAppDomainを作成
	MonoDomain* rootDomain = MonoRuntimeManager::GetInstance()->GetRootDomain();
	rootDomain = mono_domain_get();
	// TODO: ドメイン名はユニークにする必要があるかもしれない。将来的にはシーンIDを含めるなどの工夫が必要かも。


	char domainName[] = "QuickForgeSceneDomain";
	domain_ = mono_domain_create_appdomain(domainName, nullptr);

	if (!domain_) {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("Failed to create AppDomain for scene.", LogLevel::Error);
#endif
		return;
	}

	// ドメインを切り替え
	if (!mono_domain_set(domain_, false)) {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("Failed to set AppDomain.", LogLevel::Error);
#endif
		return;
	}

#ifdef QFE_OPTIMIZE_OFF
	QFE_LOG("CsharpScriptExecutor initialized with new AppDomain.");
#endif

	// アセンブリをロード
	LoadAssembly();
}

void CsharpScriptExecutor::LoadAssembly() {
	if (!domain_) {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("Domain not initialized. Cannot load assembly.", LogLevel::Error);
#endif
		return;
	}

	std::string assemblyPath = MonoRuntimeManager::GetInstance()->GetAssemblyPath();
	assembly_ = mono_domain_assembly_open(domain_, assemblyPath.c_str());

	if (!assembly_) {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("Failed to load assembly: " + assemblyPath, LogLevel::Error);
#endif
	} else {
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("Assembly loaded successfully: " + assemblyPath);
#endif
	}
}

std::vector<std::string> CsharpScriptExecutor::GetAvailableScriptClasses() const {
	std::vector<std::string> classNames;

	if (!assembly_) {
		QFE_LOG("Error: Assembly is not loaded. Cannot get script class names.", LogLevel::Error);
		return classNames;
	}

	MonoImage* image = mono_assembly_get_image(assembly_);

	if (!image) {
		QFE_LOG("Error: Could not get image from assembly.", LogLevel::Error);
		return classNames;
	}

	const MonoTableInfo* type_definitions_table = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);

	if (!type_definitions_table) {
		QFE_LOG("Error: Could not get type definitions table from image.", LogLevel::Error);
		return classNames;
	}

	int num_types = mono_table_info_get_rows(type_definitions_table);
	QFE_LOG(std::format("Scanning assembly for classes... Found {} type definitions.", num_types));

	// クラス定義テーブルをループしてクラス名と名前空間を取得
	for (int i = 0; i < num_types; i++) {
		uint32_t cols[MONO_TYPEDEF_SIZE];
		mono_metadata_decode_row(type_definitions_table, i, cols, MONO_TYPEDEF_SIZE);
		
		std::string name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
		std::string ns = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);

		// RoslynやMonoで自動生成された無効なクラスを除外
		if (name.empty() ||
			name[0] == '<' ||
			(name.find("_AnonStorey") != std::string::npos)) {
			QFE_LOG(std::format("Skipping invalid class: {}.{}", ns, name));
			continue;
		}

		// クラス名を完全修飾名で保存
		std::string full_name;

		// 名前空間がある場合は結合
		if (!ns.empty() && ns.length() > 0) {
			// 名前空間とクラス名をドットで結合
			full_name = std::string(ns) + "." + name;
			QFE_LOG(std::format("Found class: {}.{}", ns, name));
		} else {
			// 名前空間がない場合はクラス名のみ
			full_name = name;
			QFE_LOG(std::format("Found class: {}", name));
		}

		// QuickForgeEngineクラスはスクリプトとしては利用しないため除外
		if (full_name.find("QuickForgeEngine") != std::string::npos) {
			QFE_LOG(std::format("Found QuickForgeEngine class: {}", full_name));
			continue;
		}
		QFE_LOG(std::format("Found valid class: {}", full_name));
		classNames.push_back(full_name);
	}

	QFE_LOG(std::format("Finished scanning. Returning {} valid classes.", classNames.size()));
	return classNames;
}

uint32_t CsharpScriptExecutor::CreateScriptInstance(uint32_t entityId, const std::string& className) {
	if (!assembly_) {
		QFE_LOG("Assembly not loaded. Cannot create script instance.", LogLevel::Error);
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
		QFE_LOG("Class not found: " + className, LogLevel::Error);
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
				QFE_LOG(std::string("Mono Exception: ") + exceptionCStr, LogLevel::Error);
				mono_free(exceptionCStr);
			}
		}
	}

	scripts_.push_back(instance);
	QFE_LOG(std::format("Created C# script instance: {} (index: {})", className, scripts_.size() - 1));

	return static_cast<uint32_t>(scripts_.size()) - 1;
}

void QFE::CsharpScriptExecutor::CreateScriptInstance(const std::string& className) {
	if (!assembly_) {
		QFE_LOG("Assembly not loaded. Cannot create script instance.");
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
		QFE_LOG("Class not found: " + className);
		return;
	}
	MonoObject* instance = mono_object_new(domain_, monoClass);
	mono_runtime_object_init(instance);
	scripts_.push_back(instance);

	QFE_LOG(std::format("Create Instance index: {}", static_cast<uint32_t>(scripts_.size()) - 1));
	return;
}

void CsharpScriptExecutor::DeleteScriptInstance(uint32_t index) {
	if (index >= scripts_.size()) {
		QFE_LOG("Invalid script index: " + std::to_string(index), LogLevel::Error);
		return;
	}
	scripts_.erase(static_cast<size_t>(index));
	QFE_LOG("Deleted script instance at index: " + std::to_string(index));
}

void CsharpScriptExecutor::RunScriptFunction(uint32_t index, const std::string& functionName) {
	if (index >= scripts_.size()) {
		QFE_LOG("Invalid script index: " + std::to_string(index), LogLevel::Error);
		return;
	}

	MonoObject* scriptInstance = scripts_.at(index);
	MonoClass* monoClass = mono_object_get_class(scriptInstance);
	MonoMethod* method = mono_class_get_method_from_name(monoClass, functionName.c_str(), 0);

	if (!method) {
		QFE_LOG("Method not found: " + functionName, LogLevel::Warning);
		return;
	}

	MonoObject* exception = nullptr;
	mono_runtime_invoke(method, scriptInstance, nullptr, &exception);

	if (exception) {
		MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
		if (exceptionMsg) {
			char* exceptionCStr = mono_string_to_utf8(exceptionMsg);
			QFE_LOG("C# Script[" + std::to_string(index) + "]: " + exceptionCStr, LogLevel::Error);
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
	QFE_LOG("All C# scripts reset.");
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
		QFE_LOG("Failed to create new app domain.", LogLevel::Error);
		return;
	}

	if (!mono_domain_set(domain_, false)) {
		QFE_LOG("Failed to set app domain.", LogLevel::Error);
		return;
	}

	// アセンブリをリロード
	LoadAssembly();

	QFE_LOG("C# assembly reloaded.");
}

void CsharpScriptExecutor::Finalize() {
	scripts_.clear();
	assembly_ = nullptr;

	if (domain_) {
		if (MonoRuntimeManager::GetInstance()->IsInitialized()) {
			MonoDomain* rootDomain = MonoRuntimeManager::GetInstance()->GetRootDomain();
			if (rootDomain && domain_ != rootDomain) {
				mono_domain_set(rootDomain, false);
				mono_domain_unload(domain_);
			}
		}
		domain_ = nullptr;
	}

	QFE_LOG("CsharpScriptExecutor finalized.");
}
