#include "DirectX12DebugCore.h"
#ifdef _DEBUG
DirectX12DebugCore::DirectX12DebugCore() {

	debugLayer_ = std::make_unique<DebugLayer>();
	d3dResourceLeakChecker_ = std::make_unique<D3DResourceLeakChecker>();

}
#endif // _DEBUG