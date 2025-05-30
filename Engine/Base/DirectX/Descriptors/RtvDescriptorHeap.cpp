#include "RtvDescriptorHeap.h"
#include "../DirectXCommon.h"

#ifdef _DEBUG
#include "../../MyDebugLog.h"
#endif // _DEBUG


void RtvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	rtvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,numDescriptors,shaderVisible);

#ifdef _DEBUG
	DebugLog(std::format("CreateRTVDescriptorHeap: Create {} HeapSpace", numDescriptors));
#endif // _DEBUG
}

ID3D12DescriptorHeap* RtvDescriptorHeap::GetRtvDescriptorHeap() {
	return rtvDescriptorHeap_.Get();
}
