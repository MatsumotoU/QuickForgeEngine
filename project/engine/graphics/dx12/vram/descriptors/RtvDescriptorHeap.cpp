#include "RtvDescriptorHeap.h"
#include "DescriptorGenerator/DescriptorGenerator.h"
#include "CheckGenerateConfig/CheckGenerateConfig.h"

#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;
void RtvDescriptorHeap::Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) {
	QFE_LOG("-----RtvDescriptorHeap:Initialize-----");

	// ディスクリプタ生成設定の初期化
	DescriptorHeapInfo_.descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	DescriptorHeapInfo_.heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DescriptorHeapInfo_.numDescriptors = numDescriptors;
	DescriptorHeapInfo_.shaderVisible = shaderVisible;
	assert(CheckGenerateConfig::IsValid(DescriptorHeapInfo_));
	assert(device && "Device is null in RtvDescriptorHeap::Initialize");
	assert(numDescriptors > 0 && "Number of descriptors must be greater than zero.");
	assert(DescriptorHeapInfo_.descriptorSize > 0 && "Descriptor size must be greater than zero.");
	assert(DescriptorHeapInfo_.heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV && "Heap type must be RTV for RtvDescriptorHeap.");
	assert(DescriptorHeapInfo_.shaderVisible == false && "Shader visibility must be false for RTV descriptor heap.");

	device_ = device;

	// ディスクリプタヒープの生成
	DescriptorGenerator::GenerateDescriptorHeap(
		descriptorHeap_,device, DescriptorHeapInfo_);
	// 空きキューを初期化
	for (UINT i = 0; i < DescriptorHeapInfo_.numDescriptors; ++i) {
		freeDescriptors_.push(i);
	}
}

UINT RtvDescriptorHeap::GetDescriptorSize() const {
	return DescriptorHeapInfo_.descriptorSize;
}

DescriptorHandles RtvDescriptorHeap::AssignHeap(ID3D12Resource* resource,const D3D12_RENDER_TARGET_VIEW_DESC* desc) {
	// 空きスタックからディスクリプタを取得
	assert(!freeDescriptors_.empty() && "No free descriptors available.");
	UINT index = freeDescriptors_.front();

	QFE_LOG(std::format("AssignHeapIndex: {}", index));

	freeDescriptors_.pop();
	// ディスクリプタハンドルを取得
	DescriptorHandles handle;
	handle.cpuHandle_ = GetCpuDescriptorHandle(index);

	// リソースビューを生成
	device_->CreateRenderTargetView(resource, desc, handle.cpuHandle_);

	return handle;
}


