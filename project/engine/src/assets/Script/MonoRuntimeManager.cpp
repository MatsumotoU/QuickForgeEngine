#include "engine/include/assets/Script/MonoRuntimeManager.h"
#include "engine/include/assets/Script/QFElinker/CsharpOnQFELinker.h"
#include "engine/include/assets/Script/CsharpCompiler.h"
#include "engine/include/utility/String/MyString.h"
#include "engine/include/assets/AssetManager.h"

#include "engine/include/core/EngineDefines.h"

#include <Windows.h>
#include <filesystem>

using namespace QFE;

void MonoRuntimeManager::Initialize() {
	dllName_ = "MyGameScripts";

	// 実行ファイルのパスを取得
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);

	QFE_LOG("Initializing Mono JIT...");

	std::filesystem::path exeDir(path);
	exeDir = exeDir.parent_path();

	QFE_LOG("Executable Directory: " + exeDir.string());

	// Monoディレクトリの設定
	std::filesystem::path monoLibPath = exeDir / "mono" / "lib";
	std::filesystem::path monoEtcPath = exeDir / "mono" / "etc";

	// UTF-8変換
	std::string monoLibPathUtf8 = ConvertString(monoLibPath.wstring());
	std::string monoEtcPathUtf8 = ConvertString(monoEtcPath.wstring());

	QFE_LOG("Mono Lib Path: " + monoLibPath.string());
	QFE_LOG("Mono Etc Path: " + monoEtcPath.string());

	try {
		mono_set_dirs(monoLibPathUtf8.c_str(), monoEtcPathUtf8.c_str());
	}
	catch (const std::exception& e) {
		QFE_LOG(std::string("Failed to set Mono directories: ") + e.what());
		return;
	}

	// デバッグオプションの設定
	const char* options[] = {
		"--debugger-agent=transport=dt_socket,server=y,address=0.0.0.0:55555,suspend=n"
	};
	mono_jit_parse_options(sizeof(options) / sizeof(char*), (char**)options);
	QFE_LOG("Mono JIT options set for debugging.");

	// JIT初期化（プロセスごとに1回のみ）
	try {
		rootDomain_ = mono_jit_init("QuickForgeRootDomain");
	}
	catch (const std::exception& e) {
		e;
		QFE_LOG(std::string("Failed to initialize Mono JIT: ") + e.what());
		return;
	}

	if (!rootDomain_) {
		QFE_LOG("Failed to initialize Mono JIT.");
		return;
	}

	QFE_LOG("Success to initialize Mono JIT.");

	// C#スクリプトのコンパイル
	CompileScripts();

	// QFE APIの登録
	RegisterQFEAPI();
}

void MonoRuntimeManager::RegisterQFEAPI() {
	// Debug用APIの登録
	mono_add_internal_call("QuickForgeEngine.Debug::Log", 
		(const void*)CsharpOnQFELinker::Native_Debug_Log);

	// Time関連用APIの登録
	mono_add_internal_call("QuickForgeEngine.Time::GetDeltaTime", 
		(const void*)CsharpOnQFELinker::GetDeltaTime);

	// Input関連用APIの登録
	mono_add_internal_call("QuickForgeEngine.Input::GetKeyMoveDir", 
		(const void*)CsharpOnQFELinker::GetKeyMoveDir);
	mono_add_internal_call("QuickForgeEngine.Input::GetKeyTrigger", 
		(const void*)CsharpOnQFELinker::IsKeyTrigger);
	mono_add_internal_call("QuickForgeEngine.Input::GetKeyPress", 
		(const void*)CsharpOnQFELinker::IsKeyPress);
	mono_add_internal_call("QuickForgeEngine.Input::GetKeyRelease", 
		(const void*)CsharpOnQFELinker::IsKeyRelease);
	mono_add_internal_call("QuickForgeEngine.Input::GetMousePress", 
		(const void*)CsharpOnQFELinker::GetMousePress);
	mono_add_internal_call("QuickForgeEngine.Input::GetMouseTrigger", 
		(const void*)CsharpOnQFELinker::GetMouseTrigger);
	mono_add_internal_call("QuickForgeEngine.Input::GetMouseRelease", 
		(const void*)CsharpOnQFELinker::GetMouseRelease);
	mono_add_internal_call("QuickForgeEngine.Input::GetMouseScreenPos", 
		(const void*)CsharpOnQFELinker::GetMouseScreenPos);
	mono_add_internal_call("QuickForgeEngine.Input::GetMouseMoveDir", 
		(const void*)CsharpOnQFELinker::GetMouseMoveDir);
	mono_add_internal_call("QuickForgeEngine.Input::GetMouseWheelDir", 
		(const void*)CsharpOnQFELinker::GetMouseWheelDir);

	// Scene関連用APIの登録
	mono_add_internal_call("QuickForgeEngine.SceneManager::LoadScene", 
		(const void*)CsharpOnQFELinker::LoadScene);
	mono_add_internal_call("QuickForgeEngine.InternalProperty::GetEntityTags",
		(const void*)CsharpOnQFELinker::GetEntityTags);

	// Audio関連用APIの登録
	mono_add_internal_call("QuickForgeEngine.Audio::LoadSound", 
		(const void*)CsharpOnQFELinker::LoadSound);
	mono_add_internal_call("QuickForgeEngine.Audio::PlaySound", 
		(const void*)CsharpOnQFELinker::PlayQFESound);
	mono_add_internal_call("QuickForgeEngine.Audio::StopSound", 
		(const void*)CsharpOnQFELinker::StopSound);

	// Entity関連用APIの登録
	mono_add_internal_call("QuickForgeEngine.Entity::Create", 
		(const void*)CsharpOnQFELinker::CreateEntity);
    mono_add_internal_call("QuickForgeEngine.Entity::ChangeModel", 
		(const void*)CsharpOnQFELinker::ChangeModel);
    mono_add_internal_call("QuickForgeEngine.Entity::ChangeMesh", 
		(const void*)CsharpOnQFELinker::ChangeMesh);
	mono_add_internal_call("QuickForgeEngine.Entity::GetEntityFromName", 
		(const void*)CsharpOnQFELinker::GetEntityFromName);
	mono_add_internal_call("QuickForgeEngine.Entity::Destroy",
		(const void*)CsharpOnQFELinker::DeleteEntity);

	// Transform関連用APIの登録
	mono_add_internal_call("QuickForgeEngine.InternalProperty::GetTransforms", 
		(const void*)CsharpOnQFELinker::GetTransforms);
	mono_add_internal_call("QuickForgeEngine.InternalProperty::SetTransforms",
		(const void*)CsharpOnQFELinker::SetTransforms);

	mono_add_internal_call("QuickForgeEngine.TransformInternal::GetTranslate", 
		(const void*)CsharpOnQFELinker::GetTransformTranslate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::SetTranslate", 
		(const void*)CsharpOnQFELinker::SetTransformTranslate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::GetRotate", 
		(const void*)CsharpOnQFELinker::GetTransformRotate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::SetRotate", 
		(const void*)CsharpOnQFELinker::SetTransformRotate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::GetScale", 
		(const void*)CsharpOnQFELinker::GetTransformScale);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::SetScale", 
		(const void*)CsharpOnQFELinker::SetTransformScale);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::Translate", 
		(const void*)CsharpOnQFELinker::Translate);
	mono_add_internal_call("QuickForgeEngine.TransformInternal::Rotate", 
		(const void*)CsharpOnQFELinker::Rotate);

	// Collision関連用APIの登録
	mono_add_internal_call("QuickForgeEngine.InternalProperty::GetCollisionEnterEntityIDs", 
		(const void*)CsharpOnQFELinker::GetCollisionEnterEntityIds);
	mono_add_internal_call("QuickForgeEngine.InternalProperty::GetCollisionStayEntityIDs", 
		(const void*)CsharpOnQFELinker::GetCollisionStayEntityIds);

	QFE_LOG("QFE C# API registered successfully.");
}

void MonoRuntimeManager::CompileScripts() {
	// スクリプトディレクトリのパスを取得
	std::string scriptsDir = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	if (!scriptsDir.empty() && scriptsDir.back() != '/' && scriptsDir.back() != '\\') {
		scriptsDir += '/';
	}

	// DLL名の設定
	std::string dllName = dllName_;

	// csprojファイルの生成
	std::string projectName = dllName + ".csproj";
	std::string csprojPath = scriptsDir + projectName;
	QFE::GenerateCsproj(scriptsDir, csprojPath);

	// DLL出力パスの設定
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	std::filesystem::path exeDir = std::filesystem::path(path).parent_path();
	std::string dllPath = (exeDir / (dllName + ".dll")).string();

	// C#スクリプトのコンパイル
	try {
		QFE::CompileCSharpProject(csprojPath, dllPath);
		QFE_LOG("C# scripts compiled successfully.");
	}
	catch (const std::exception& e) {
		e;
		QFE_LOG(std::string("Failed to compile C# scripts: ") + e.what(), LogLevel::Error);
	}
}

void QFE::MonoRuntimeManager::CreateCSProject(const std::string& projectName)
{
	// スクリプトディレクトリのパスを取得
	std::string scriptsDir = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	if (!scriptsDir.empty() && scriptsDir.back() != '/' && scriptsDir.back() != '\\') {
		scriptsDir += '/';
	}

	// csprojファイルの生成
	std::string csprojPath = scriptsDir + projectName;
	QFE::GenerateCsproj(scriptsDir, csprojPath);
}

std::string MonoRuntimeManager::GetAssemblyPath() const {
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	std::filesystem::path exeDir = std::filesystem::path(path).parent_path();
	return (exeDir / (dllName_ + ".dll")).string();
}

void MonoRuntimeManager::Finalize() {
	// Monoランタイム全体をクリーンアップ
	if (rootDomain_) {
		QFE_LOG("Cleaning up Mono JIT...");
		mono_jit_cleanup(rootDomain_);
		rootDomain_ = nullptr;
	}
}
