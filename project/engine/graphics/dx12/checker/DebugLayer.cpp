#include "DebugLayer.h"

using namespace QFE::GRAPHIC::INTERNAL;
#ifdef QFE_OPTIMIZE_OFF
DebugLayer::DebugLayer() {
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		debugController_->EnableDebugLayer();
		debugController_->SetEnableGPUBasedValidation(TRUE);
		debugController_->SetEnableSynchronizedCommandQueueValidation(TRUE);
	}
}
#else
DebugLayer::DebugLayer() {}
#endif

