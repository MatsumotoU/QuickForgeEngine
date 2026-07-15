#include "Vector4.h"
#include <assert.h>
#include <cmath>

#include "Vector3.h"
#include "../matrix/Matrix4x4.h"

using namespace QFE::MATH;

float Vector4::Length() const {
	return sqrt(x * x + y * y + z * z + w * w);
}

Vector4 Vector4::Normalize() const {
	Vector4 result = {};

	if (this->Length() == 0.0f) {
		assert(false);
	}

	result.x = x / this->Length();
	result.y = y / this->Length();
	result.z = z / this->Length();
	result.w = w / this->Length();

	return result;
}

Vector4 Vector4::EulerTransform(const Vector4& vector, const Matrix4x4& matrix) {
	Vector4 result;
	result.x = vector.x * matrix.Get(0, 0) + vector.y * matrix.Get(1, 0) + vector.z * matrix.Get(2, 0) + vector.w * matrix.Get(3, 0);
	result.y = vector.x * matrix.Get(0, 1) + vector.y * matrix.Get(1, 1) + vector.z * matrix.Get(2, 1) + vector.w * matrix.Get(3, 1);
	result.z = vector.x * matrix.Get(0, 2) + vector.y * matrix.Get(1, 2) + vector.z * matrix.Get(2, 2) + vector.w * matrix.Get(3, 2);
	result.w = vector.x * matrix.Get(0, 3) + vector.y * matrix.Get(1, 3) + vector.z * matrix.Get(2, 3) + vector.w * matrix.Get(3, 3);
	return result;
}

Vector3 Vector4::xyz() const {
	Vector3 result{};
	result.x = this->x;
	result.y = this->y;
	result.z = this->z;
	return result;
}

Vector4 Vector4::Leap(const Vector4& v1, const Vector4& v2, float t) {
	Vector4 result{};
	result.x = v1.x * (1.0f - t) + v2.x * t;
	result.y = v1.y * (1.0f - t) + v2.y * t;
	result.z = v1.z * (1.0f - t) + v2.z * t;
	result.w = v1.w * (1.0f - t) + v2.w * t;
	return result;
}
