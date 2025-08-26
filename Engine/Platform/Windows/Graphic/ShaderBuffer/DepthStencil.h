#pragma once
#include <d3d12.h>
#include <stdint.h>
#include <wrl.h>

#include "Platform/Windows/Graphic/DirectXCommon/Descriptors/DsvDescriptorHeap.h"

class DepthStencil final {
public:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
		ID3D12Device* device, int32_t width, int32_t height,DsvDescriptorHeap* dsvHeap);

	D3D12_DEPTH_STENCIL_DESC GetDepthStencilDesc();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDsvDescriptorHeap();

private:
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_;
	D3D12_RESOURCE_DESC resourceDesc_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
	DescriptorHandles stencileHandle_;
};