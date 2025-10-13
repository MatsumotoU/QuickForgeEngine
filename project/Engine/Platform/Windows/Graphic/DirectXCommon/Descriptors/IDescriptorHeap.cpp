#include "IDescriptorHeap.h"
#include "AppUtility/DirectX/GenerateDiscriptorHandle.h"
#include <cassert>

ID3D12DescriptorHeap* IDescriptorHeap::GetDescriptorHeap() const {
	return descriptorHeap_.Get();
}

ID3D12DescriptorHeap* const* IDescriptorHeap::GetDescriptorHeapAddressOf() const {
	return descriptorHeap_.GetAddressOf();
}

D3D12_CPU_DESCRIPTOR_HANDLE IDescriptorHeap::GetCPUDescriptorHandleForHeapStart() const {
	return GenerateDescriptorHandle::GetCpuDescriptorHandle(
		descriptorHeap_.Get(), descriptorGenerateConfig_.descriptorSize,0);
}

D3D12_GPU_DESCRIPTOR_HANDLE IDescriptorHeap::GetGPUDescriptorHandleForHeapStart() const {
	return GenerateDescriptorHandle::GetGpuDescriptorHandle(
		descriptorHeap_.Get(), descriptorGenerateConfig_.descriptorSize, 0);
}