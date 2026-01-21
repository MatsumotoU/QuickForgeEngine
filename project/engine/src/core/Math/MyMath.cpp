/**
 * @file MyMath.cpp
 * @brief 数学ユーティリティ関数の実装
 */

#include "engine/include/core/Math/MyMath.h"
#include <cassert>

/**
 * @brief 線形補間
 */
float MyMath::Lerp(float a, float b, float t) {
    return a * t + b * (1.0f - t);
}

/**
 * @brief 球面線形補間(Slerp)の簡易版?
 * TODO: 実際は線形補間のような計算になっているため、Slerpとしての数学的正確性を確認
 */
float MyMath::Slerp(float from, float to, float t) {
	if (fabsf(from - to) > 0.0001f) {
		return (from * (1.0f - t)) + (to * t);
	}
	return 0.0f;
}

/** @brief 指数平滑化を用いた簡易的な補間 */
void MyMath::SimpleEaseIn(float* value, float endValue, float transitionSpeed) {
	if (value == nullptr) {
		assert(false && "value is nullptr.");
		return;
	}

	*value += (endValue - *value) * transitionSpeed;

	if (fabsf(*value - endValue) <= 0.01f) {
		*value = endValue;
	}
}

/** @brief 指数平滑化を用いた簡易的な補間 (戻り値版) */
float MyMath::SimpleEaseIn(float from, float to, float transitionSpeed) {
	float value = from;
	value += (to - value) * transitionSpeed;
	if (fabsf(value - to) <= 0.01f) {
		return to;
	}
	return value;
}

/** @brief 度からラジアンへ変換 */
float MyMath::DegreesToRadians(float degrees) {
	return degrees * 3.14159265358979323846f / 180.0f;
}

/** @brief 球とAABBの最近接点を求める */
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
