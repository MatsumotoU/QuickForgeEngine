#include "Vector4.h"
#include <assert.h>
#include <cmath>

#include "Vector3.h"

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
