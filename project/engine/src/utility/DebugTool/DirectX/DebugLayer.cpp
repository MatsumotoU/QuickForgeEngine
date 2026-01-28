#include "engine/include/utility/DebugTool/DirectX/DebugLayer.h"
using namespace QFE;
#ifdef QFE_OPTIMIZE_OFF
DebugLayer::DebugLayer() {
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		debugController_->EnableDebugLayer();
		debugController_->SetEnableGPUBasedValidation(TRUE);
		debugController_->SetEnableSynchronizedCommandQueueValidation(TRUE);
	}
}
#endif // QFE_OPTIMIZE_OFF
