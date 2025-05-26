#pragma once
#include <wrl.h>
#include <d3d12.h>

class SrvDescriptorHeap final {
public:
	void Initialize(ID3D12Device* device, UINT numDescriptors, bool shaderVisible);

public:
	ID3D12DescriptorHeap* GetSrvDescriptorHeap();

private:
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap_;
};