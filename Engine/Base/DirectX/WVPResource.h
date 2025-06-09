#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "../../Math/Matrix/Matrix4x4.h"
class DirectXCommon;
struct TransformationMatrix;

class WVPResource final {
public:
	void Initialize(DirectXCommon* dxCommon,uint32_t totalResources);

public:
	ID3D12Resource* GetWVPResource();

public:
	void SetWorldMatrix(const Matrix4x4& worldMatrix, uint32_t index);
	void SetWVPMatrix(const Matrix4x4& wvpMatrix, uint32_t index);

public:
	TransformationMatrix* wvpData_;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	
};