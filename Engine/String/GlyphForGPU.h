#pragma once
#include "Math/Vector/Vector2.h"
#include "Math/Vector/Vector4.h"
#include "Math/Matrix/Matrix4x4.h"

#include <cstdint>

struct alignas(16) GlyphForGPU
{
	Matrix4x4 WVP;
	Vector4 texcorad;
};