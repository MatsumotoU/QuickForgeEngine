#include "engine/include/core/Bridge/EngineBridgeProvider.h"
#include <stdexcept> 

void QFE::BRIDGE::EngineBridgeProvider::SetUpBridge(std::unique_ptr<IEngineBridge> bridge) {
	bridgeInstance_ = std::move(bridge);
}

void QFE::BRIDGE::EngineBridgeProvider::FinalizeBridge() {
	bridgeInstance_.reset();
}

QFE::IEngineBridge* QFE::BRIDGE::EngineBridgeProvider::Get() const { 
	if(bridgeInstance_ == nullptr) {
		throw std::runtime_error("EngineBridge instance is not set up.");
	}
	return bridgeInstance_.get();
}
