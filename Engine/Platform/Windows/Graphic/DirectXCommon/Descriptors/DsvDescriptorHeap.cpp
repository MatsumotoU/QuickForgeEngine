#include "DsvDescriptorHeap.h"
#include "DescriptorGenerator/DescriptorGenerator.h"
#include "CheckGenerateConfig/CheckGenerateConfig.h"
#include "AppUtility/DirectX/GenerateDiscriptorHandle.h"
#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void DsvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	assert(device);
	assert(numDescriptors > 0);
	assert(!shaderVisible);// DSVはシェーダーから参照しない
	device_ = device;
	
	// ディスクリプタ生成設定の初期化
	descriptorGenerateConfig_.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	descriptorGenerateConfig_.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descriptorGenerateConfig_.numDescriptors = numDescriptors;
	descriptorGenerateConfig_.shaderVisible = shaderVisible;
	assert(CheckGenerateConfig::IsValid(descriptorGenerateConfig_));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = numDescriptors;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	// ディスクリプタヒープの生成
	DescriptorGenerator::GenerateDescriptorHeap(
		descriptorHeap_, device, descriptorGenerateConfig_);
	// 空きキューを初期化
	for (UINT i = 0; i < descriptorGenerateConfig_.numDescriptors; ++i) {
		freeDescriptors_.push(i);
	}
}

DescriptorHandles DsvDescriptorHeap::AssignHeap(ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc) {
	// 空きスタックからディスクリプタを取得
	assert(!freeDescriptors_.empty() && "No free descriptors available.");
	UINT index = freeDescriptors_.front();
#ifdef _DEBUG
	DebugLog(std::format("AssignHeapIndex: {}", index));
#endif // _DEBUG
	freeDescriptors_.pop();
	// ディスクリプタハンドルを取得
	DescriptorHandles handle;
	handle.cpuHandle_ =
		GenerateDescriptorHandle::GetCpuDescriptorHandle(descriptorHeap_.Get(), descriptorGenerateConfig_.descriptorSize, index);
	// リソースビューを生成
	device_->CreateDepthStencilView(resource, desc, handle.cpuHandle_);
	return handle;
}

UINT DsvDescriptorHeap::GetDescriptorSize() const {
	return descriptorGenerateConfig_.descriptorSize;
}
