#include "SrvDescriptorHeap.h"
#include "../DirectXCommon.h"

#ifdef _DEBUG
#include "../../MyDebugLog.h"
#endif // _DEBUG

void SrvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	srvDescriptorHeap_ = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, numDescriptors, shaderVisible);
	DebugLog(std::format("CreateSRVDescriptorHeap: Create {} HeapSpace", numDescriptors));
}

ID3D12DescriptorHeap* SrvDescriptorHeap::GetSrvDescriptorHeap() {
	return srvDescriptorHeap_.Get();
}
