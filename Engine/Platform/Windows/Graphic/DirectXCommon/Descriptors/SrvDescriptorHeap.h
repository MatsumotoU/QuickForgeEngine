#pragma once
#include "IDescriptorHeap.h"
#include <cassert>

class SrvDescriptorHeap final : public IDescriptorHeap {
public:
	SrvDescriptorHeap() = default;
	~SrvDescriptorHeap() override = default;
	void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible) override;
	[[nodiscard]] DescriptorHandles AssignHeap(
		ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);
	UINT GetDescriptorSize() const override;
};