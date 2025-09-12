#pragma once
#include "Core/Math/Matrix/Matrix4x4.h"
#include "Core/Math/Vector/Vector4.h"

struct alignas(16) ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};