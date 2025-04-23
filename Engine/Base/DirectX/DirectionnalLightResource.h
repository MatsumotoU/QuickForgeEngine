#pragma once
#include <d3d12.h>
#include <wrl.h>
class DirectXCommon;
struct DirectionalLight;

class DirectionnalLightResource {
public:
	DirectionnalLightResource(DirectXCommon* dxCommon);

public:
	ID3D12Resource* GetDirectionnalLightResource();
	DirectionalLight* directionnalLightData_;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> directionnalLightResource_;
	
};