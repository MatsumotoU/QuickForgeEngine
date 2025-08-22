#pragma once
#include "RtvDescriptorHeap.h"
#include "SrvDescriptorHeap.h"

class DescriptorHeapManager final {
public:
	DescriptorHeapManager() = default;
	~DescriptorHeapManager() = default;
	void Initialize(ID3D12Device* device);
	[[nodiscard]] DescriptorHandles AssignRtvHeap(ID3D12Resource* resource, const D3D12_RENDER_TARGET_VIEW_DESC* desc);
	[[nodiscard]] DescriptorHandles AssignSrvHeap(ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	[[nodiscard]] UINT GetRtvDescriptorSize() const;
	[[nodiscard]] UINT GetSrvDescriptorSize() const;

private:
	const uint32_t kMaxRtvDescriptors = 128; // 最大RTVディスクリプタ数
	const uint32_t kMaxSrvDescriptors = 512; // 最大SRVディスクリプタ数

	RtvDescriptorHeap rtvDescriptorHeap;
	SrvDescriptorHeap srvDescriptorHeap;
};