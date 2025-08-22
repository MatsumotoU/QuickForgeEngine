#include "DescriptorHeapManager.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void DescriptorHeapManager::Initialize(ID3D12Device* device) {
#ifdef _DEBUG
	DebugLog("-----DescriptorHeapManager:Initialize-----\n");
#endif // _DEBUG
	rtvDescriptorHeap.Initialize(device, kMaxRtvDescriptors, false);
	srvDescriptorHeap.Initialize(device, kMaxSrvDescriptors, true);
}

DescriptorHandles DescriptorHeapManager::AssignRtvHeap(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc) {
	return rtvDescriptorHeap.AssignHeap(resource, desc);
}

DescriptorHandles DescriptorHeapManager::AssignSrvHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
	return srvDescriptorHeap.AssignHeap(resource, desc);
}

UINT DescriptorHeapManager::GetRtvDescriptorSize() const {
	return rtvDescriptorHeap.GetDescriptorSize();
}

UINT DescriptorHeapManager::GetSrvDescriptorSize() const {
	return srvDescriptorHeap.GetDescriptorSize();
}
