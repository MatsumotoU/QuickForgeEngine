#pragma once
#include "Math/Matrix/Matrix4x4.h"
#include "Math/Matrix/Matrix3x3.h"
#include "Math/Vector/Vector4.h"
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector2.h"

namespace MyGameMath {
	Vector3 MakeDirection(const Vector3& rotate);
	template<class T>
	bool InRange(T value, T min, T max) {
		return (value >= min && value <= max);
	}
}