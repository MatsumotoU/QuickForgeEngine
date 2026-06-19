#include "DescriptorHeap.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC::INTERNAL;

void DescriptorHeap::Create(ID3D12Device* device, DescriptorHeapInfo info) {
	// すでに生成されているかどうかの確認
	if(isCreated_) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap::Create failed: already created", SystemError::Abort);
		return;
	}

	// 引数の確認
	if(device == nullptr) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap::Create failed: device is null",SystemError::Abort);
		return;
	}
	if (!info.CheckValid()) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap::Create failed: invalid DescriptorHeapInfo",SystemError::Abort);
		return;
	}
	DescriptorHeapInfo_ = info;

	// ディスクリタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = info.heapType;
	descriptorHeapDesc.NumDescriptors = info.numDescriptors;
	descriptorHeapDesc.Flags = info.shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(descriptorHeap_.GetAddressOf()));
	if (FAILED(hr)) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap::Create failed: failed to create descriptor heap", SystemError::Abort);
		return;
	}
	// ヒープの空きスロットを初期化
	for (UINT i = 0; i < DescriptorHeapInfo_.numDescriptors; ++i) {
		freeDescriptors_.push(i);
	}

	isCreated_ = true;
}	

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCpuDescriptorHandle(uint32_t index) const {
	CheckCreated();
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (DescriptorHeapInfo_.descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuDescriptorHandle(uint32_t index) const {
	CheckCreated();
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (DescriptorHeapInfo_.descriptorSize * index);
	return handleGPU;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUDescriptorHandleForHeapStart() const {
	CheckCreated();
	return GetCpuDescriptorHandle(0);
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUDescriptorHandleForHeapStart() const {
	CheckCreated();
	return GetGpuDescriptorHandle(0);
}

UINT DescriptorHeap::GetDescriptorSize() const {
	CheckCreated();
	return DescriptorHeapInfo_.descriptorSize;
}

uint32_t DescriptorHeap::GetNextFreeDescriptorIndex() {
	CheckCreated();
	if (freeDescriptors_.empty()) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap::GetNextFreeDescriptorIndex failed: no free descriptors available", SystemError::Abort);
		return UINT32_MAX;
	}
	uint32_t index = freeDescriptors_.front();
	freeDescriptors_.pop();
	return index;
}

ID3D12DescriptorHeap* DescriptorHeap::GetDescriptorHeap() const {
	CheckCreated();
	return descriptorHeap_.Get();
}

void DescriptorHeap::CheckCreated() const {
	if (!isCreated_) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap is not created", SystemError::Abort);
	}
}
