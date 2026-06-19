#include "DescriptorHeap.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void DescriptorHeap::Create(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = config.heapType;
	descriptorHeapDesc.NumDescriptors = config.numDescriptors;
	descriptorHeapDesc.Flags = config.shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(heap.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));
}
