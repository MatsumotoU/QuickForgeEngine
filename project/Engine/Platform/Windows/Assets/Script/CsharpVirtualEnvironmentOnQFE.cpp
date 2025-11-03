#include "CsharpVirtualEnvironmentOnQFE.h"

void CsharpVirtualEnvironmentOnQFE::Initialize() {
	domain_ = mono_jit_init("MyAppDomain");
	assembly_ = mono_domain_assembly_open(domain_, "ScriptsAssembly.dll");
}

void CsharpVirtualEnvironmentOnQFE::Finalize() {
	mono_jit_cleanup(domain_);
}
