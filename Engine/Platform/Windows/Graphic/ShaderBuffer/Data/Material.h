#pragma once
#include "pch.h"
#include "Core/Math/Matrix/Matrix4x4.h"
#include "Core/Math/Vector/Vector4.h"

/// <summary>
/// ピクセルシェーダー用マテリアル
/// Vector4 color;
/// int32_t enableLighting;
/// float padding[3];
/// Matrix4x4 uvTransform;
/// </summary>
struct alignas(16) Material {
	Vector4 color = {1.0f,1.0f,1.0f,1.0f};
	int32_t enableLighting = true;
	float padding[3] = {0.0f,0.0f,0.0f};
	Matrix4x4 uvTransform = Matrix4x4::MakeIndentity4x4();
};