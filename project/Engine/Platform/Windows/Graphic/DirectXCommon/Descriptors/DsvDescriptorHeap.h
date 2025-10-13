#pragma once
#include "IDescriptorHeap.h"
#include <cassert>
class DsvDescriptorHeap final : public IDescriptorHeap {
public:
	DsvDescriptorHeap() = default;
	~DsvDescriptorHeap() override = default;
	void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) override;
	[[nodiscard]] DescriptorHandles AssignHeap(
		ID3D12Resource* resource, const D3D12_DEPTH_STENCIL_VIEW_DESC* desc);
	UINT GetDescriptorSize() const override;
};