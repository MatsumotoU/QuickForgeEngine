#pragma once
#include <cstdint>
#include "Math/Matrix/Matrix4x4.h"
#include "Math/Vector/Vector4.h"

/// <summary>
/// ピクセルシェーダー用マテリアル
/// Vector4 color;
/// int32_t enableLighting;
/// float padding[3];
/// Matrix4x4 uvTransform;
/// </summary>
struct alignas(16) Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;

	Material() : color(1.0f, 1.0f, 1.0f, 1.0f)
		, enableLighting(1)
		, padding{ 0.0f, 0.0f, 0.0f }
		, uvTransform(Matrix4x4::MakeIndentity4x4()) {
	}
};