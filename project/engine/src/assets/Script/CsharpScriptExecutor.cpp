#include "engine/include/assets/Script/CsharpScriptExecutor.h"
#include "engine/include/assets/Script/MonoRuntimeManager.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/assets/Script/Data/CsharpComponent.h"

#include "engine/include/core/EngineDefines.h"

#include <format>

using namespace QFE;

CsharpScriptExecutor::CsharpScriptExecutor() {}

CsharpScriptExecutor::~CsharpScriptExecutor() {
	Finalize();
}

void CsharpScriptExecutor::ResetScripts() {
	ResetGameLogicManager();
	QFE_LOG("All C# scripts reset.");
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

	// GameLogicManagerのインスタンスを作成
	ResetGameLogicManager();
}

void QFE::CsharpScriptExecutor::InitializeGameLogic(EntityManager* entityManager)
{
	// エンティティマネージャからScriptComponentを持つものに対して、インスタンスを作成するように指示する
	entityManager->Each<CsharpComponent>([this](uint32_t entityId, CsharpComponent& csharpComponent) {
		if (!csharpComponent.csharpHandles_.empty()) {
			CreateScriptInstance(entityId, csharpComponent.csharpHandles_[0].className_);
		}
		});

	if (gameLogicManagerInstance_ && gameLogicInitializeMethod_) {
		mono_runtime_invoke(gameLogicInitializeMethod_, gameLogicManagerInstance_, nullptr, nullptr);
	}
	else {
		QFE_LOG("GameLogicManager is not properly initialized. Cannot call Initialize.", LogLevel::Error);
	}
}

void CsharpScriptExecutor::FrameStart() {
	if (gameLogicManagerInstance_ && gameLogicFrameStartMethod_) {
		mono_runtime_invoke(gameLogicFrameStartMethod_, gameLogicManagerInstance_, nullptr, nullptr);
	}
	else {
		QFE_LOG("GameLogicManager is not properly initialized. Cannot call FrameStart.", LogLevel::Error);
	}
}

void CsharpScriptExecutor::Update() {
	if (gameLogicManagerInstance_ && gameLogicUpdateMethod_) {
		mono_runtime_invoke(gameLogicUpdateMethod_, gameLogicManagerInstance_, nullptr, nullptr);
	}
}

void CsharpScriptExecutor::FrameEnd() {
	if (gameLogicManagerInstance_ && gameLogicFrameEndMethod_) {
		mono_runtime_invoke(gameLogicFrameEndMethod_, gameLogicManagerInstance_, nullptr, nullptr);
	}
	else {
		QFE_LOG("GameLogicManager is not properly initialized. Cannot call FrameEnd.", LogLevel::Error);
	}
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

	if(!gameLogicManagerInstance_) {
		QFE_LOG("GameLogicManager instance is not initialized. Cannot create script instance.", LogLevel::Error);
		return 0;
	}

	MonoObject* exception = nullptr;
	void* args[2];
	args[0] = &entityId;
	args[1] = const_cast<char*>(className.c_str());
	mono_runtime_invoke(gameLogicCreateScriptInstanceMethod_, gameLogicManagerInstance_, args, &exception);

	if (exception) {
		QFE_REPORT_USER_ERROR(std::format("Exception occurred while creating script instance for class '{}'.", className), UserError::DeveloperError);
		return 0;
	}

	return 0;
}

void CsharpScriptExecutor::DeleteScriptInstance(uint32_t index) {
	
}

void CsharpScriptExecutor::ReloadAssembly() {
	// スクリプトインスタンスをクリア
	gameLogicManagerInstance_ = nullptr;
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

	// GameLogicManagerのインスタンスを再作成
	ResetGameLogicManager();

	QFE_LOG("C# assembly reloaded.");
}

void QFE::CsharpScriptExecutor::ResetGameLogicManager()
{
	MonoImage* image = mono_assembly_get_image(assembly_);
	gameLogicManagerClass_ = mono_class_from_name(image, "QuickForgeEngine", "GameLogicManager");
	if (gameLogicManagerClass_) {
		gameLogicManagerInstance_ = mono_object_new(domain_, gameLogicManagerClass_);
		mono_runtime_object_init(gameLogicManagerInstance_);

		gameLogicInitializeMethod_ = mono_class_get_method_from_name(gameLogicManagerClass_, "InitializeAll", 0);
		gameLogicCreateScriptInstanceMethod_ = mono_class_get_method_from_name(gameLogicManagerClass_, "CreateScriptInstance", 2);
		gameLogicUpdateMethod_ = mono_class_get_method_from_name(gameLogicManagerClass_, "UpdateAll", 0);
		gameLogicFrameStartMethod_ = mono_class_get_method_from_name(gameLogicManagerClass_, "FrameStart", 0);
		gameLogicFrameEndMethod_ = mono_class_get_method_from_name(gameLogicManagerClass_, "FrameEnd", 0);

		QFE_LOG("GameLogicManager instance created successfully.");
	}
	else {
		QFE_LOG("Failed to find GameLogicManager class in assembly.", LogLevel::Error);
	}
}

void CsharpScriptExecutor::Finalize() {
	gameLogicManagerInstance_ = nullptr;
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
