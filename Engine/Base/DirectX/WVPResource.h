#pragma once
#include <d3d12.h>
#include <wrl.h>

class DirectXCommon;
struct TransformationMatrix;

class WVPResource {
public:
	void Initialize(DirectXCommon* dxCommon);

public:
	ID3D12Resource* GetWVPResource();
	TransformationMatrix* wvpData_;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	
};