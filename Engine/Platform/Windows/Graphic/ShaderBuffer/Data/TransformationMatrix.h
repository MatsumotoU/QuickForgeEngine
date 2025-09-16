#pragma once
#include "Core/Math/Matrix/Matrix4x4.h"

struct alignas(16) TransformationMatrix {
	Matrix4x4 WVP = Matrix4x4::MakeIndentity4x4();
	Matrix4x4 World = Matrix4x4::MakeIndentity4x4();
};