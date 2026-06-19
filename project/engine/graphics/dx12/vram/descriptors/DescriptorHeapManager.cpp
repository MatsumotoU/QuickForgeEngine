#include "DescriptorHeapManager.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void DescriptorHeapManager::Initialize(ID3D12Device* device) {
	QFE_LOG("-----DescriptorHeapManager:Initialize-----\n");
	// DescriptorHeapInfoの設定
	DescriptorHeapInfo rtvHeapInfo{};
	rtvHeapInfo.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapInfo.numDescriptors = kMaxRtvDescriptors;
	rtvHeapInfo.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	rtvHeapInfo.shaderVisible = false;
	DescriptorHeapInfo srvHeapInfo{};
	srvHeapInfo.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapInfo.numDescriptors = kMaxSrvDescriptors;
	srvHeapInfo.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	srvHeapInfo.shaderVisible = true;
	DescriptorHeapInfo dsvHeapInfo{};
	dsvHeapInfo.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapInfo.numDescriptors = kMaxDsvDescriptors;
	dsvHeapInfo.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	dsvHeapInfo.shaderVisible = false;

	// DescriptorHeapの生成
	descriptorHeaps_.emplace(DescriptorHeapType::RTV, DescriptorHeap());
	descriptorHeaps_.emplace(DescriptorHeapType::SRV, DescriptorHeap());
	descriptorHeaps_.emplace(DescriptorHeapType::DSV, DescriptorHeap());
	descriptorHeaps_[DescriptorHeapType::RTV].Create(device, rtvHeapInfo);
	descriptorHeaps_[DescriptorHeapType::SRV].Create(device, srvHeapInfo);
	descriptorHeaps_[DescriptorHeapType::DSV].Create(device, dsvHeapInfo);
}

DescriptorHandles DescriptorHeapManager::AssignRtvHeap(
	ID3D12Device* device, ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc) {

	// 空いているディスクリタのインデックスを取得
	uint32_t index = descriptorHeaps_[DescriptorHeapType::RTV].GetNextFreeDescriptorIndex();
	// ヒープにディスクリタを割り当てる
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeaps_[DescriptorHeapType::RTV].GetCpuDescriptorHandle(index);

	// ディスクリタを作成
	device->CreateRenderTargetView(resource, desc, cpuHandle);

	// DescriptorHandlesを作成
	DescriptorHandles handles{};
	handles.cpuHandle_ = cpuHandle;
	// RTVはGPUからアクセスできないため、GPUハンドルは無効にする

	return handles;
}

DescriptorHandles DescriptorHeapManager::AssignSrvHeap(
	ID3D12Device* device, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {

	// 空いているディスクリタのインデックスを取得
	uint32_t index = descriptorHeaps_[DescriptorHeapType::SRV].GetNextFreeDescriptorIndex();
	// ヒープにディスクリタを割り当てる
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeaps_[DescriptorHeapType::SRV].GetCpuDescriptorHandle(index);
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = descriptorHeaps_[DescriptorHeapType::SRV].GetGpuDescriptorHandle(index);

	// ディスクリタを作成
	device->CreateShaderResourceView(resource, &desc, cpuHandle);

	// DescriptorHandlesを作成
	DescriptorHandles handles{};
	handles.cpuHandle_ = cpuHandle;
	handles.gpuHandle_ = gpuHandle;
	return handles;
}

DescriptorHandles DescriptorHeapManager::AssignDsvHeap(
	ID3D12Device* device, ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc) {

	// 空いているディスクリタのインデックスを取得
	uint32_t index = descriptorHeaps_[DescriptorHeapType::DSV].GetNextFreeDescriptorIndex();
	// ヒープにディスクリタを割り当てる
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = descriptorHeaps_[DescriptorHeapType::DSV].GetCpuDescriptorHandle(index);

	// ディスクリタを作成
	device->CreateDepthStencilView(resource, desc, cpuHandle);

	// DescriptorHandlesを作成
	DescriptorHandles handles{};
	handles.cpuHandle_ = cpuHandle;
	// DSVはGPUからアクセスできないため、GPUハンドルは無効にする
	return handles;
}