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
	dsvDescriptorHeap.Initialize(device, kMaxDsvDescriptors, false);
}

DescriptorHandles DescriptorHeapManager::AssignRtvHeap(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc) {
	return rtvDescriptorHeap.AssignHeap(resource, desc);
}

DescriptorHandles DescriptorHeapManager::AssignSrvHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
	return srvDescriptorHeap.AssignHeap(resource, desc);
}

DescriptorHandles DescriptorHeapManager::AssignDsvHeap(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc) {
	return dsvDescriptorHeap.AssignHeap(resource,desc);
}

UINT DescriptorHeapManager::GetRtvDescriptorSize() const {
	return rtvDescriptorHeap.GetDescriptorSize();
}

UINT DescriptorHeapManager::GetSrvDescriptorSize() const {
	return srvDescriptorHeap.GetDescriptorSize();
}

UINT DescriptorHeapManager::GetDsvDescriptorSize() const {
	return dsvDescriptorHeap.GetDescriptorSize();
}

RtvDescriptorHeap* DescriptorHeapManager::GetRtvDescriptorHeap() {
	return &rtvDescriptorHeap;
}

SrvDescriptorHeap* DescriptorHeapManager::GetSrvDescriptorHeap() {
	return &srvDescriptorHeap;
}

DsvDescriptorHeap* DescriptorHeapManager::GetDsvDescriptorHeap() {
	return &dsvDescriptorHeap;
}

ID3D12DescriptorHeap* DescriptorHeapManager::GetRtvDescriptorHeapAddress() { 
	return rtvDescriptorHeap.GetDescriptorHeap(); 
}

ID3D12DescriptorHeap* DescriptorHeapManager::GetSrvDescriptorHeapAddress() {
	return srvDescriptorHeap.GetDescriptorHeap();
}

ID3D12DescriptorHeap* DescriptorHeapManager::GetDsvDescriptorHeapAddress() {
	return dsvDescriptorHeap.GetDescriptorHeap();
}

ID3D12DescriptorHeap* const* DescriptorHeapManager::GetRtvDescriptorHeapAddressOf() {
	return rtvDescriptorHeap.GetDescriptorHeapAddressOf();
}

ID3D12DescriptorHeap* const* DescriptorHeapManager::GetSrvDescriptorHeapAddressOf() {
	return srvDescriptorHeap.GetDescriptorHeapAddressOf();
}

ID3D12DescriptorHeap* const* DescriptorHeapManager::GetDsvDescriptorHeapAddressOf() {
	return dsvDescriptorHeap.GetDescriptorHeapAddressOf();
}
