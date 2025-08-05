#include "RtvDescriptorHeap.h"
#include "../DirectXCommon.h"

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

// TODO: メモリ領域の可視化
// TODO: メモリ選定機能の追加、外部から変更できないようにする

void RtvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	rtvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV,numDescriptors,shaderVisible);

#ifdef _DEBUG
	DebugLog(std::format("CreateRTVDescriptorHeap: Create {} HeapSpace", numDescriptors));
#endif // _DEBUG
}

ID3D12DescriptorHeap* RtvDescriptorHeap::GetRtvDescriptorHeap() {
	return rtvDescriptorHeap_.Get();
}
