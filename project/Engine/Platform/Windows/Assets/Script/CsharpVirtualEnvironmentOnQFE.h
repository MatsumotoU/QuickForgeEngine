#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include "Utility/memory/SparseSets.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <Windows.h> 
#include <string> 
#include <filesystem> 

class CsharpVirtualEnvironmentOnQFE final:public Singleton<CsharpVirtualEnvironmentOnQFE> {
	friend class Singleton<CsharpVirtualEnvironmentOnQFE>;
public:
	void Initialize();
	void CreateCSProject(std::string& projectName);
	void CompileScripts();
	void OpenCSharpProjectInVSCode();
	void LinkQFEAPIToMono();
	void LoadAssembly();
	void CreateScriptInstance(const std::string& className);
	void RunScriptFunction(uint32_t index,const std::string& functionName);
	void ReloadAssembly();
	void Finalize();

private:
	MonoDomain* root_domain_ = nullptr;
	MonoDomain* domain_ = nullptr;
	MonoAssembly* assembly_ = nullptr;
	std::vector<MonoObject*> scripts_;
};