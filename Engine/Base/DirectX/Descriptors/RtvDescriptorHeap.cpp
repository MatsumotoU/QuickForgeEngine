#include "RtvDescriptorHeap.h"
#include "../DirectXCommon.h"

#ifdef _DEBUG
#include "../../MyDebugLog.h"
#endif // _DEBUG


void RtvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	rtvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,numDescriptors,shaderVisible);
	DebugLog(std::format("CreateRTVDescriptorHeap: Create {} HeapSpace",numDescriptors));
}

ID3D12DescriptorHeap* RtvDescriptorHeap::GetRtvDescriptorHeap() {
	return rtvDescriptorHeap_.Get();
}
