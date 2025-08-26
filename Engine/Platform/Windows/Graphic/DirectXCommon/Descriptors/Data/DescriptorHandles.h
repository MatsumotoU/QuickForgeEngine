#pragma once
#include <d3d12.h>
struct DescriptorHandles final {
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;
};