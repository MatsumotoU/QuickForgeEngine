#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <stack>
#include "Data/DescriptorGenerateConfig.h"
#include "Data/DescriptorHandles.h"

class IDescriptorHeap {
public:
	virtual ~IDescriptorHeap() = default;
	virtual void Initialize(ID3D12Device* device,UINT numDescriptors,bool shaderVisible) = 0;
	ID3D12DescriptorHeap* GetDescriptorHeap() const;
	[[nodiscard]] D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandleForHeapStart() const;
	[[nodiscard]] D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const;

	virtual UINT GetDescriptorSize() const = 0;

protected:
	ID3D12Device* device_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
	std::stack<UINT> freeDescriptors_; // スタックを使用して空きディスクリプタを管理
	DescriptorGenerateConfig descriptorGenerateConfig_; // ディスクリプタ生成設定
};