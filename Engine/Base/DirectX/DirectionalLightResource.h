#pragma once
#include <d3d12.h>
#include <wrl.h>
class DirectXCommon;
struct DirectionalLight;

class DirectionalLightResource {
public:
	void Initialize(DirectXCommon* dxCommon);

public:
	ID3D12Resource* GetDirectionalLightResource();
	DirectionalLight* directionalLightData_;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource_;
	
};