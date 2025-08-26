#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <queue>
#include "Data/DescriptorGenerateConfig.h"
#include "Data/DescriptorHandles.h"

class IDescriptorHeap {
public:
	virtual ~IDescriptorHeap() = default;
	virtual void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) = 0;
	[[nodiscard]] ID3D12DescriptorHeap* GetDescriptorHeap() const;
	[[nodiscard]] ID3D12DescriptorHeap* const* GetDescriptorHeapAddressOf() const;
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const;
	[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const;

	virtual UINT GetDescriptorSize() const = 0;

protected:
	ID3D12Device* device_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	std::queue<uint32_t> freeDescriptors_;
	DescriptorGenerateConfig descriptorGenerateConfig_; // ディスクリプタ生成設定
};