#include "DirectX12DebugCore.h"

DirectX12DebugCore::DirectX12DebugCore() {
#ifdef _DEBUG
	debugLayer_ = std::make_unique<DebugLayer>();
	d3dResourceLeakChecker_ = std::make_unique<D3DResourceLeakChecker>();
#endif // _DEBUG
}