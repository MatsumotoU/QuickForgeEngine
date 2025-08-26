#pragma once
#include <d3d12.h>
struct DescriptorGenerateConfig {
	D3D12_DESCRIPTOR_HEAP_TYPE heapType;
	UINT numDescriptors;
	UINT descriptorSize;
	bool shaderVisible;
};