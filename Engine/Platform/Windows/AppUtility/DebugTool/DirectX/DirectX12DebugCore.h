#pragma once
#define NOMINMAX
#include <memory>
#include "D3DResourceLeakChecker.h"
#include "DebugLayer.h"

class D3DResourceLeakChecker;
class DebugLayer;

class DirectX12DebugCore final {
public:
	DirectX12DebugCore();

private:
	std::unique_ptr<DebugLayer> debugLayer_;
	std::unique_ptr<D3DResourceLeakChecker> d3dResourceLeakChecker_;
};