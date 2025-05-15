#pragma once
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;

class PostProcess {
	void Initialize(DirectXCommon* dxCommon);

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> postProcessRtvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> postProcessSrvDescriptorHeap_;
};