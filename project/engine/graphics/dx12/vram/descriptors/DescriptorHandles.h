#pragma once
#include <d3d12.h>
namespace QFE::GRAPHIC::INTERNAL {
	/// @brief ディスクリタヒープから割り当てられたCPUとGPUのディスクリタハンドルを保持する構造体
	struct DescriptorHandles final {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;
	};
}