#pragma once
#include <d3d12.h>
#include <stdint.h>
#include <wrl.h>

class DirectXCommon;
class WinApp;

// TODO: DescriptorHeapを引きはがす

class DepthStencil final {
public:
	void Initialize(WinApp* winApp, DirectXCommon* dxCommon);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, int32_t width, int32_t height);

public:
	D3D12_DEPTH_STENCIL_DESC* GetDepthStencilDesc();
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> GetDsvDescriptorHeap();

private:
	DirectXCommon* dxCommon_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_;
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc_;
	D3D12_RESOURCE_DESC resourceDesc_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_;
};