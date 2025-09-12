#pragma once
#include "Math/Matrix/Matrix4x4.h"

struct alignas(16) TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};