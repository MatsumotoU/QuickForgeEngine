#include "DescriptorHeap.h"
#include "EngineDefines.h"

using namespace QFE::GRAPHIC;

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
	nextFreeDescriptorIndex_ = 0;

	isCreated_ = true;
	isShaderVisible_ = info.shaderVisible;
}
std::vector<DescriptorHandles> QFE::GRAPHIC::DescriptorHeap::CreateDescriptorHandles(uint32_t count) {
	std::vector<DescriptorHandles> handles;
	handles.reserve(count);
	for(uint32_t i = 0; i < count; ++i) {
		if (nextFreeDescriptorIndex_ >= DescriptorHeapInfo_.numDescriptors) {
			QFE_REPORT_SYSTEM_ERROR("DescriptorHeap::CreateDescriptorHandles failed: no free descriptors available", SystemError::Abort);
			return {};
		}
		uint32_t index = nextFreeDescriptorIndex_;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = GetCpuDescriptorHandle(index);
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle {};
		if (DescriptorHeapInfo_.shaderVisible) {
			gpuHandle = GetGpuDescriptorHandle(index);
		}
		handles.push_back({ cpuHandle, gpuHandle });
		++nextFreeDescriptorIndex_;
	}
	return handles;
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
	if (nextFreeDescriptorIndex_ >= DescriptorHeapInfo_.numDescriptors) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap::GetNextFreeDescriptorIndex failed: no free descriptors available", SystemError::Abort);
		return UINT32_MAX;
	}
	uint32_t index = nextFreeDescriptorIndex_;
	++nextFreeDescriptorIndex_;
	return index;
}

ID3D12DescriptorHeap* DescriptorHeap::GetDescriptorHeap() const {
	CheckCreated();
	return descriptorHeap_.Get();
}

const bool QFE::GRAPHIC::DescriptorHeap::IsShaderVisible() const {
	CheckCreated();	
	return isShaderVisible_;
}

void DescriptorHeap::CheckCreated() const {
	if (!isCreated_) {
		QFE_REPORT_SYSTEM_ERROR("DescriptorHeap is not created", SystemError::Abort);
	}
}
