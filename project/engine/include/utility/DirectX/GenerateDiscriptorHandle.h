#pragma once
#include <d3d12.h>
#include <cstdint>
class GenerateDescriptorHandle {
public:
	/// <summary>
	/// 繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝ｼ繝偵・繝励°繧韻PU繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝ｼ繝上Φ繝峨Ν繧貞叙蠕励＠縺ｾ縺吶・
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	[[nodiscard]] static D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// 繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝ｼ繝偵・繝励°繧烏PU繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝ｼ繝上Φ繝峨Ν繧貞叙蠕励＠縺ｾ縺吶・
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	[[nodiscard]] static D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
};
