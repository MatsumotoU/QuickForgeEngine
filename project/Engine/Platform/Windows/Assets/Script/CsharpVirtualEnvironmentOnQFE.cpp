#include "CsharpVirtualEnvironmentOnQFE.h"
#include "CsharpCmpiler.h"

#include <windows.h>

#include "Assets/AssetManager.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void CsharpVirtualEnvironmentOnQFE::Initialize() {
	// 実行ファイルのパスを取得
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	
	// 実行ファイルのディレクトリを取得
	std::filesystem::path exeDir(path);
	exeDir = exeDir.parent_path();

	// Monoのlibとetcディレクトリへのパスを構築 (exeと同じ階層のmonoフォルダ内)
	std::filesystem::path monoLibPath = exeDir / "mono" / "lib";
	std::filesystem::path monoEtcPath = exeDir / "mono" / "etc";

	// Monoランタイムにライブラリと設定ファイルの場所を教える
	mono_set_dirs(monoLibPath.string().c_str(), monoEtcPath.string().c_str());

	// ルートドメインを初期化 (プログラム終了時まで保持)
	root_domain_ = mono_jit_init("QuickForgeRootDomain");
	if (!root_domain_) {
#ifdef _DEBUG
		DebugLog("Failed to initialize Mono JIT.");
#endif // _DEBUG
		return;
	}

	// 最初のアセンブリロードを実行
	ReloadAssembly();
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

// C#の QuickForgeEngine.Debug.Log(string message) に対応するC++ラッパー関数
static void Native_Debug_Log(MonoString* message)
{
    // MonoString* を C++で扱える const char* に変換します
    char* utf8_message = mono_string_to_utf8(message);

    // あなたのC++のログ機能を呼び出します
    DebugLog(utf8_message);

    // mono_string_to_utf8で確保されたメモリを解放します
    mono_free(utf8_message);
}

void CsharpVirtualEnvironmentOnQFE::LinkQFEAPIToMono() {
	// 正しいメソッド名 "名前空間.クラス名::メソッド名" を指定し、ラッパー関数を登録します
	mono_add_internal_call("QuickForgeEngine.Debug::Log", (const void*)Native_Debug_Log);
}

void CsharpVirtualEnvironmentOnQFE::LoadAssembly() {
	if (!domain_) {
#ifdef _DEBUG
		DebugLog("Mono domain not initialized.");
#endif // _DEBUG
		return;
	}

	if (assembly_) {
#ifdef _DEBUG
		DebugLog("Assembly already loaded.");
#endif // _DEBUG
		return;
	}

	std::string dllPath = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts") + "bin/Debug/netstandard2.0/CSharpScripts.dll";
	assembly_ = mono_domain_assembly_open(domain_, dllPath.c_str());
	if (!assembly_) {
#ifdef _DEBUG
		DebugLog("Failed to load assembly: " + dllPath);
#endif // _DEBUG
	}
	return;
}

void CsharpVirtualEnvironmentOnQFE::CreateScriptInstance(const std::string& className) {
	if (!assembly_) {
#ifdef _DEBUG
		DebugLog("Assembly not loaded. Cannot create script instance.");
#endif // _DEBUG
		return;
	}
	MonoImage* image = mono_assembly_get_image(assembly_);
	MonoClass* monoClass = mono_class_from_name(image, "", className.c_str());
	if (!monoClass) {
#ifdef _DEBUG
		DebugLog("Class not found: " + className);
#endif // _DEBUG
		return;
	}
	MonoObject* instance = mono_object_new(domain_, monoClass);
	mono_runtime_object_init(instance);
	scripts_.push_back(instance);

#ifdef _DEBUG
	DebugLog(std::format("Create Instance index: {}", static_cast<uint32_t>(scripts_.size())-1));
#endif // _DEBUG

}

void CsharpVirtualEnvironmentOnQFE::RunScriptFunction(uint32_t index, const std::string& functionName) {
	MonoObject* scriptInstance = scripts_.at(index);
	MonoClass* monoClass = mono_object_get_class(scriptInstance);
	MonoMethod* method = mono_class_get_method_from_name(monoClass, functionName.c_str(), 0);
	if (!method) {
#ifdef _DEBUG
		DebugLog("Method not found: " + functionName);
#endif // _DEBUG
		return;
	}
	MonoObject* exception = nullptr;
	mono_runtime_invoke(method, scriptInstance, nullptr, &exception);
	if (exception) {
		MonoString* exceptionMsg = mono_object_to_string(exception, nullptr);
		if (exceptionMsg) {
			char* exceptionCStr = mono_string_to_utf8(exceptionMsg);
#ifdef _DEBUG
			DebugLog(std::string("Mono Exception: ") + exceptionCStr);
#endif
			mono_free(exceptionCStr);
		}
	}
}

void CsharpVirtualEnvironmentOnQFE::ReloadAssembly() {
	// 既存のスクリプトインスタンスへの参照をクリア
	scripts_.clear();
	assembly_ = nullptr;

	// 既存のスクリプトドメインをアンロード
	if (domain_) {
		// ルートドメインはアンロードしない
		if (domain_ != root_domain_) {
			// アンロードする前に、カレントドメインを安全なルートドメインに戻す
			mono_domain_set(root_domain_, false);
			mono_domain_unload(domain_);
		}
		domain_ = nullptr;
	}

	// --- 新しいドメインを作成してアセンブリをロード ---
	char domain_name[] = "QuickForgeScriptDomain";
	domain_ = mono_domain_create_appdomain(domain_name, nullptr);
	if (!domain_) {
#ifdef _DEBUG
		DebugLog("Failed to create new app domain.");
#endif
		return;
	}

	// 新しく作成したドメインを現在のドメインとして設定
	if (!mono_domain_set(domain_, false)) {
#ifdef _DEBUG
		DebugLog("Failed to set app domain.");
#endif
		return;
	}

	// スクリプトを再コンパイル
	CompileScripts();

	// APIを再度リンク
	LinkQFEAPIToMono();

	// 新しいドメインにアセンブリをロード
	LoadAssembly();
}

void CsharpVirtualEnvironmentOnQFE::Finalize() {
	// スクリプトドメインをアンロード
	if (domain_ && domain_ != root_domain_) {
		// カレントドメインをルートに戻してからアンロードするのが安全
		mono_domain_set(root_domain_, false);
		mono_domain_unload(domain_);
		domain_ = nullptr;
	}

	// Monoランタイム全体をクリーンアップ
	if (root_domain_) {
		mono_jit_cleanup(root_domain_);
		root_domain_ = nullptr;
	}
}
