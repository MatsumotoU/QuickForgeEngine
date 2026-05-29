#include "DebugLayer.h"

#ifdef QFE_OPTIMIZE_OFF
using namespace QFE::GRAPHIC::INTERNAL;

DebugLayer::DebugLayer() {
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		debugController_->EnableDebugLayer();
		debugController_->SetEnableGPUBasedValidation(TRUE);
		debugController_->SetEnableSynchronizedCommandQueueValidation(TRUE);
	}
}
#endif // QFE_OPTIMIZE_OFF
