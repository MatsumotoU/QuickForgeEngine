#pragma once
#include <d3d12.h>
#include <wrl.h>

#include "../../Math/Matrix/Matrix4x4.h"
class DirectXCommon;
struct TransformationMatrix;

class WVPResource {
public:
	void Initialize(DirectXCommon* dxCommon);

public:
	ID3D12Resource* GetWVPResource();

public:
	void SetWorldMatrix(const Matrix4x4& worldMatrix);
	void SetWVPMatrix(const Matrix4x4& wvpMatrix);

public:
	TransformationMatrix* wvpData_;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
	
};