#pragma once
#include <d3d12.h>
#include <wrl.h>
#include "../../Math/Vector/Vector4.h"
#include "../../Math/Matrix/Matrix4x4.h"

class DirectXCommon;
struct Material;

class MaterialResource {
public:
	void Initialize(DirectXCommon* dxCommon, const Vector4& color = { 1.0f,1.0f,1.0f,1.0f }, bool enebleLighting = true);

public:
	ID3D12Resource* GetMaterial();

public:
	void SetUvTransformMatrix(const Matrix4x4& uvTransformMatrix);

public:
	Material* materialData_;

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	
};