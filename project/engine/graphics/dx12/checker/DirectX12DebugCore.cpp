#include "DirectX12DebugCore.h"

#ifdef QFE_OPTIMIZE_OFF
using namespace QFE::GRAPHIC::INTERNAL;

DirectX12DebugCore::DirectX12DebugCore() {

	debugLayer_ = std::make_unique<DebugLayer>();
	d3dResourceLeakChecker_ = std::make_unique<D3DResourceLeakChecker>();

}
#endif // QFE_OPTIMIZE_OFF
