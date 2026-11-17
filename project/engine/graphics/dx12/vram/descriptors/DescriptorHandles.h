#pragma once
#include <d3d12.h>
#include <cstdint>
namespace QFE::GRAPHIC {
	/// @brief ディスクリタヒープから割り当てられたCPUとGPUのディスクリタハンドルを保持する構造体
	struct DescriptorHandles final {
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;
		uint32_t textureIndex = UINT32_MAX; ///< テクスチャ配列内の実際の位置。テクスチャ以外は無効。
	};
}
