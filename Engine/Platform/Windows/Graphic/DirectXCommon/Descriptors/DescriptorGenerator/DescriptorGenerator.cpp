#include "DescriptorGenerator.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#include <cassert>
#endif // _DEBUG

void DescriptorGenerator::GenerateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap, ID3D12Device* device, const DescriptorGenerateConfig& config) {
	assert(!heap && "Already generated");
#ifdef _DEBUG
	DebugLog("CreateDescriptorHeap");
	DebugLog(std::format("NumDescriptors: {}, ShaderVisible: {}",
		config.numDescriptors, config.shaderVisible ? "true" : "false"));
#endif // _DEBUG

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = config.heapType;
	descriptorHeapDesc.NumDescriptors = config.numDescriptors;
	descriptorHeapDesc.Flags = config.shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(heap.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));
}
