#pragma once
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "engine/include/utility/memory/SparseSets.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <Windows.h> 
#include <string> 
#include <filesystem> 
#include <vector>

/**
 * C#スクリプトとC++の管理、Monoの管理、スクリプトのコンパイルをすべてこなすクラス.
 */
class CsharpVirtualEnvironmentOnQFE final:public Singleton<CsharpVirtualEnvironmentOnQFE> {
	friend class Singleton<CsharpVirtualEnvironmentOnQFE>;
public:
	/// @brief 初期化処理.
	void Initialize();
	/// @brief スクリプト環境のリセット.
	void ResetScripts();
	/// @brief C#プロジェクトの作成.
	void CreateCSProject(std::string& projectName);
	/// @brief スクリプトのコンパイル.
	void CompileScripts();
	/// @brief VSCodeでC#プロジェクトを開く.
	void OpenCSharpProjectInVSCode();
	/// @brief MonoとQFEAPIの連携.
	void LinkQFEAPIToMono();
	/// @brief アセンブリの読み込み.
	void LoadAssembly();
	/// @brief 利用可能なスクリプトクラス名の取得.
	std::vector<std::string> GetAvailableScriptClasses() const;
	/// @brief スクリプトインスタンスの生成.
	uint32_t CreateScriptInstance(const std::string& className);
	/// @brief スクリプトインスタンスの生成(エンティティID付き).
	uint32_t CreateScriptInstance(uint32_t entityId, const std::string& className);
	/// @brief スクリプト関数の実行.
	void RunScriptFunction(uint32_t index,const std::string& functionName);
	/// @brief アセンブリのリロード.
	void ReloadAssembly();
	/// @brief すべてのスクリプトで指定した関数を実行.
	void RunAllScriptsFunction(const std::string& functionName);
	/// @brief 終了処理.
	void Finalize();

private:
	MonoDomain* root_domain_ = nullptr;
	MonoDomain* domain_ = nullptr;
	MonoAssembly* assembly_ = nullptr;
	std::vector<MonoObject*> scripts_;
};
