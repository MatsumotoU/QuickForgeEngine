#include "DirectX12DebugCore.h"

using namespace QFE::GRAPHIC;
#ifdef QFE_OPTIMIZE_OFF
DirectX12DebugCore::DirectX12DebugCore() {

	debugLayer_ = std::make_unique<DebugLayer>();
	d3dResourceLeakChecker_ = std::make_unique<D3DResourceLeakChecker>();

}
#else
DirectX12DebugCore::DirectX12DebugCore() = default;
#endif // QFE_OPTIMIZE_OFF
