#include "DirectX12DebugCore.h"
#include "D3DResourceLeakChecker.h"

DirectX12DebugCore::DirectX12DebugCore() {
	resourceLeakChecker_ = new D3DResourceLeakChecker();
}

DirectX12DebugCore::~DirectX12DebugCore() {
	delete resourceLeakChecker_;
	resourceLeakChecker_ = nullptr;
}
