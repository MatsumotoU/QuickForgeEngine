#include "SrvDescriptorHeap.h"

#include "DescriptorGenerator/DescriptorGenerator.h"
#include "CheckGenerateConfig/CheckGenerateConfig.h"
#include "AppUtility/DirectX/GenerateDiscriptorHandle.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void SrvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
#ifdef _DEBUG
	DebugLog("-----SrvDescriptorHeap:Initialize-----\n");
#endif // _DEBUG
	// ディスクリプタ生成設定の初期化
	descriptorGenerateConfig_.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorGenerateConfig_.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorGenerateConfig_.numDescriptors = numDescriptors;
	descriptorGenerateConfig_.shaderVisible = shaderVisible;
	assert(CheckGenerateConfig::IsValid(descriptorGenerateConfig_));
	assert(device && "Device is null in SrvDescriptorHeap::Initialize");
	assert(numDescriptors > 0 && "Number of descriptors must be greater than zero.");
	assert(descriptorGenerateConfig_.descriptorSize > 0 && "Descriptor size must be greater than zero.");
	assert(descriptorGenerateConfig_.heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV && "Heap type must be Srv for SrvDescriptorHeap.");
	assert(descriptorGenerateConfig_.shaderVisible == true && "Shader visibility must be true for Srv descriptor heap.");

	device_ = device; // デバイスを保存

	// ディスクリプタヒープの生成
	DescriptorGenerator::GenerateDescriptorHeap(
		descriptorHeap_, device, descriptorGenerateConfig_);
	// 空きスタックを初期化
	for (UINT i = 0; i < descriptorGenerateConfig_.numDescriptors; ++i) {
		freeDescriptors_.push(i);
	}
}

UINT SrvDescriptorHeap::GetDescriptorSize() const {
	return descriptorGenerateConfig_.descriptorSize;
}

DescriptorHandles SrvDescriptorHeap::AssignHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {
#ifdef _DEBUG
	DebugLog("-----RtvDescriptorHeap:AssignHeap-----\n");
#endif // _DEBUG
	// 空きスタックからディスクリプタを取得
	assert(!freeDescriptors_.empty() && "No free descriptors available.");
	UINT index = freeDescriptors_.top();
	freeDescriptors_.pop();
	// ディスクリプタハンドルを取得
	DescriptorHandles handle;
	handle.cpuHandle_ =
		GenerateDescriptorHandle::GetCpuDescriptorHandle(descriptorHeap_.Get(), descriptorGenerateConfig_.descriptorSize, index);
	handle.gpuHandle_ =
		GenerateDescriptorHandle::GetGpuDescriptorHandle(descriptorHeap_.Get(), descriptorGenerateConfig_.descriptorSize, index);

	// リソースビューを生成
	device_->CreateShaderResourceView(resource, &desc, handle.cpuHandle_);
	return handle;
}
