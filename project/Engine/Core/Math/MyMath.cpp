#include "MyMath.h"

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
	Vector3 closestPoint;
	closestPoint.x = std::max(aabb.min.x, std::min(sphere.center.x, aabb.max.x));
	closestPoint.y = std::max(aabb.min.y, std::min(sphere.center.y, aabb.max.y));
	closestPoint.z = std::max(aabb.min.z, std::min(sphere.center.z, aabb.max.z));
	return closestPoint;
}
