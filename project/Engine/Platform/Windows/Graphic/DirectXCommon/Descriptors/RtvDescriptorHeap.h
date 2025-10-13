#pragma once
#include "IDescriptorHeap.h"
#include <cassert>

class RtvDescriptorHeap final : public IDescriptorHeap {
public:
	RtvDescriptorHeap() = default;
	~RtvDescriptorHeap() override = default;
	void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) override;
	[[nodiscard]] DescriptorHandles AssignHeap(
		ID3D12Resource* resource,const D3D12_RENDER_TARGET_VIEW_DESC* desc);
	UINT GetDescriptorSize() const override;
};