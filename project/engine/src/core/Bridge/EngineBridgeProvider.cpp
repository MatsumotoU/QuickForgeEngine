#include "engine/include/core/Bridge/EngineBridgeProvider.h"
#include "engine/include/core/EngineDefines.h"

#include <stdexcept> 

void QFE::BRIDGE::EngineBridgeProvider::SetUpBridge(std::unique_ptr<IEngineBridge> bridge) {
	bridgeInstance_ = std::move(bridge);
}

void QFE::BRIDGE::EngineBridgeProvider::FinalizeBridge() {
	bridgeInstance_.reset();
}

QFE::IEngineBridge* QFE::BRIDGE::EngineBridgeProvider::Get() const { 
	if(bridgeInstance_ == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("EngineBridge instance is not set up.",SystemError::Abort);
	}
	return bridgeInstance_.get();
}
