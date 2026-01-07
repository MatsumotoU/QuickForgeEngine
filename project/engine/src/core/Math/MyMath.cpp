#include "engine/include/core/Math/MyMath.h"

float MyMath::Leap(float a, float b, float t) {
    return a * t + b * (1.0f - t);
}

float MyMath::Slerp(float from, float to, float t) {
	if (fabsf(from - to) > 0.0001f) {
		return (from * (1.0f - t)) + (to * t);
	}
	return 0.0f;
}

void MyMath::SimpleEaseIn(float* value, float endValue, float transitionSpeed) {
	*value += (endValue - *value) * transitionSpeed;

	if (fabsf(*value - endValue) <= 0.01f) {
		*value = endValue;
	}
}

float MyMath::SimpleEaseIn(float from, float to, float transitionSpeed) {
	float value = from;
	value += (to - value) * transitionSpeed;
	if (fabsf(value - to) <= 0.01f) {
		return to;
	}
	return value;
}

float MyMath::DegreesToRadians(float degrees) {
	return degrees * 3.14159265358979323846f / 180.0f;
}

Vector3 MyMath::ClosestPoint(const Sphere& sphere, const AABB& aabb) {
	Vector3 halfSize = aabb.size * 0.5f;
	Vector3 min = aabb.center - halfSize;
	Vector3 max = aabb.center + halfSize;
	Vector3 closestPoint;
	closestPoint.x = std::max(min.x, std::min(sphere.center.x, max.x));
	closestPoint.y = std::max(min.y, std::min(sphere.center.y, max.y));
	closestPoint.z = std::max(min.z, std::min(sphere.center.z, max.z));
	return closestPoint;
}
