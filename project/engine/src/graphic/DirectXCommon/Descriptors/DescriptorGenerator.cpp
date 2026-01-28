#include "engine/include/graphic/DirectXCommon/Descriptors/DescriptorGenerator/DescriptorGenerator.h"
#include <cassert>
#ifdef QFE_OPTIMIZE_OFF
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // QFE_OPTIMIZE_OFF
using namespace QFE;
void DescriptorGenerator::GenerateDescriptorHeap(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& heap, ID3D12Device* device, const DescriptorGenerateConfig& config) {
	assert(!heap && "Already generated");
#ifdef QFE_OPTIMIZE_OFF
	DebugLog("CreateDescriptorHeap");
	DebugLog(std::format("NumDescriptors: {}, ShaderVisible: {}",
		config.numDescriptors, config.shaderVisible ? "true" : "false"));
#endif // QFE_OPTIMIZE_OFF

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = config.heapType;
	descriptorHeapDesc.NumDescriptors = config.numDescriptors;
	descriptorHeapDesc.Flags = config.shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(heap.GetAddressOf()));
	hr;
	assert(SUCCEEDED(hr));
}
