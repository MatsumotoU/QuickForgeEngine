#pragma once
#include "Math/Vector/Vector3.h"
#include "Math/Matrix/Matrix4x4.h"
#include "Math/Transform.h"

class RotateToDir final {
public:
	static Vector3 GetRotateToDir(const Vector3& rotate);
};
