#include "DebugLayer.h"
#ifdef _DEBUG
DebugLayer::DebugLayer() {
	debugController_ = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController_)))) {
		debugController_->EnableDebugLayer();
		debugController_->SetEnableGPUBasedValidation(TRUE);
		debugController_->SetEnableSynchronizedCommandQueueValidation(TRUE);
	}
}
#endif // _DEBUG