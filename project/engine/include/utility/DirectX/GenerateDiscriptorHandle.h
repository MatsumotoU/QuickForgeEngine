#pragma once
#include <d3d12.h>
#include <cstdint>
class GenerateDescriptorHandle {
public:
	/// <summary>
	/// チE��スクリプターヒ�EプからCPUチE��スクリプターハンドルを取得します、E
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	[[nodiscard]] static D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
	/// <summary>
	/// チE��スクリプターヒ�EプからGPUチE��スクリプターハンドルを取得します、E
	/// </summary>
	/// <param name="descriptorHeap"></param>
	/// <param name="descriptorSize"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	[[nodiscard]] static D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index);
};
