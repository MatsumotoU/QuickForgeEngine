#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include "Utility/memory/SparseSets.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

class CsharpVirtualEnvironmentOnQFE final:public Singleton<CsharpVirtualEnvironmentOnQFE> {
	friend class Singleton<CsharpVirtualEnvironmentOnQFE>;
public:
	void Initialize();
	void Finalize();

private:
	MonoDomain* domain_;
	MonoAssembly* assembly_;
	SparseSet<MonoObject> scripts_;
};