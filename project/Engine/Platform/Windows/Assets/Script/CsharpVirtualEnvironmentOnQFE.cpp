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

	// ドメインを初期化
	domain_ = mono_jit_init("MyAppDomain");
}

void CsharpVirtualEnvironmentOnQFE::CreateCSProject(std::string& projectName) {
	AssetManager* assetManager = AssetManager::GetInstance();
	std::string scriptsDir = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	std::string projectDir = assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Project");
	std::string csprojPath = scriptsDir + projectName + ".csproj";
	std::string outputDllPath = projectDir + projectName + ".dll";
	GenerateCsproj(scriptsDir, csprojPath);
	//CompileCSharpProject(csprojPath, outputDllPath);
}

void CsharpVirtualEnvironmentOnQFE::CompileScripts() {
	AssetManager* assetManager = AssetManager::GetInstance();
	assetManager->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
}

void CsharpVirtualEnvironmentOnQFE::OpenCSharpProjectInVSCode() {
	std::string scriptDir = AssetManager::GetInstance()->GetResourceDirectoryManager()->GetResourceDirectory("Scripts");
	std::string command = "code \"" + scriptDir + "\"";
	system(command.c_str());
}

void CsharpVirtualEnvironmentOnQFE::LoadAssembly(const std::string& assemblyPath) {
	assembly_ = mono_domain_assembly_open(domain_, assemblyPath.c_str());
	if (!assembly_) {
#ifdef _DEBUG
		DebugLog("Failed to load assembly: " + assemblyPath);
#endif // _DEBUG
	}
	return;
}

void CsharpVirtualEnvironmentOnQFE::Finalize() {
	mono_jit_cleanup(domain_);
}
