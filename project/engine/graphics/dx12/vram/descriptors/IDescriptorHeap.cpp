#include "IDescriptorHeap.h"
#include "GenerateDescriptorHandle.h"
#include <cassert>
using namespace QFE::GRAPHIC::INTERNAL;

ID3D12DescriptorHeap* IDescriptorHeap::GetDescriptorHeap() const {
	return descriptorHeap_.Get();
}

ID3D12DescriptorHeap* const* IDescriptorHeap::GetDescriptorHeapAddressOf() const {
	return descriptorHeap_.GetAddressOf();
}

D3D12_CPU_DESCRIPTOR_HANDLE IDescriptorHeap::GetCPUDescriptorHandleForHeapStart() const {
	return GenerateDescriptorHandle::GetCpuDescriptorHandle(
		descriptorHeap_.Get(), DescriptorHeapInfo_.descriptorSize,0);
}

D3D12_GPU_DESCRIPTOR_HANDLE IDescriptorHeap::GetGPUDescriptorHandleForHeapStart() const {
	return GenerateDescriptorHandle::GetGpuDescriptorHandle(
		descriptorHeap_.Get(), DescriptorHeapInfo_.descriptorSize, 0);
}

D3D12_CPU_DESCRIPTOR_HANDLE QFE::GRAPHIC::INTERNAL::IDescriptorHeap::GetCpuDescriptorHandle(uint32_t index) const {
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (DescriptorHeapInfo_.descriptorSize * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE QFE::GRAPHIC::INTERNAL::IDescriptorHeap::GetGpuDescriptorHandle(uint32_t index) const {
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (DescriptorHeapInfo_.descriptorSize * index);
	return handleGPU;
}


