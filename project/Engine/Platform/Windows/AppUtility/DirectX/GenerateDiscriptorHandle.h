#pragma once
#include <d3d12.h>
#include <cstdint>
class GenerateDescriptorHandle {
public:
	/// <summary>
	/// ディスクリプターヒープからCPUディスクリプターハンドルを取得します。
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	[[nodiscard]] static D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// ディスクリプターヒープからGPUディスクリプターハンドルを取得します。
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	[[nodiscard]] static D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
};