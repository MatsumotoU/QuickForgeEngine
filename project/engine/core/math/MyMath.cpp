#include "MyMath.h"
#include <cassert>

float QFE::MATH::Lerp(float a, float b, float t) {
    return a * t + b * (1.0f - t);
}

float QFE::MATH::Slerp(float from, float to, float t) {
	if (fabsf(from - to) > 0.0001f) {
		return (from * (1.0f - t)) + (to * t);
	}
	return 0.0f;
}


void QFE::MATH::SimpleEaseIn(float* value, float endValue, float transitionSpeed) {
	if (value == nullptr) {
		assert(false && "value is nullptr.");
		return;
	}

	*value += (endValue - *value) * transitionSpeed;

	if (fabsf(*value - endValue) <= 0.01f) {
		*value = endValue;
	}
}

float QFE::MATH::SimpleEaseIn(float from, float to, float transitionSpeed) {
	float value = from;
	value += (to - value) * transitionSpeed;
	if (fabsf(value - to) <= 0.01f) {
		return to;
	}
	return value;
}

/** @brief 度からラジアンへ変換 */
float QFE::MATH::DegreesToRadians(float degrees) {
	return degrees * 3.14159265358979323846f / 180.0f;
}

/** @brief 球とAABBの最近接点を求める */
QFE::MATH::Vector3 QFE::MATH::ClosestPoint(const Sphere& sphere, const AABB& aabb) {
	Vector3 halfSize = aabb.size * 0.5f;
	Vector3 min = aabb.center - halfSize;
	Vector3 max = aabb.center + halfSize;
	Vector3 closestPoint;
	closestPoint.x = std::max(min.x, std::min(sphere.center.x, max.x));
	closestPoint.y = std::max(min.y, std::min(sphere.center.y, max.y));
	closestPoint.z = std::max(min.z, std::min(sphere.center.z, max.z));
	return closestPoint;
}

bool QFE::MATH::IsPrime(uint64_t number)
{
	if (number <= 1) return false;
	if (number <= 3) return true;
	if (number % 2 == 0 || number % 3 == 0) return false;

	for (uint64_t i = 5; i * i <= number; i += 6) {
		if (number % i == 0 || number % (i + 2) == 0) {
			return false;
		}
	}
	return true;
}