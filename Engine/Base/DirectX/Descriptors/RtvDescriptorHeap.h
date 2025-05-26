#pragma once
#include <wrl.h>
#include <d3d12.h>

class RtvDescriptorHeap final {
public:
	void Initialize(ID3D12Device* device,UINT numDescriptors, bool shaderVisible);

public:
	ID3D12DescriptorHeap* GetRtvDescriptorHeap();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_;
};