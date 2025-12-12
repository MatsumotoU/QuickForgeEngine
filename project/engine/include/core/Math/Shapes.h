#pragma once
#include "Vector/Vector3.h"

/// 球体
struct Sphere
{
	Vector3 center;
	float radius;
	int subdivision;
};
/// 軸並行境界ボックス
struct AABB
{
	Vector3 min;
	Vector3 max;
};
/// 有向境界ボックス
struct OBB
{
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};
/// 板
struct Plane
{
	Vector3 normal; // 法線
	float distance; // 距離

	float DistanceToPoint(const Vector3& point);
};
/// 三角形
struct Triangle {
	Vector3 vertices[3];
};
/// 直線
struct Line
{
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差分ベクトル
};
/// 半直線
struct Ray
{
	Vector3 origin; //!< 始点
	Vector3 diff; //!< 終点への差分ベクトル
};
/// 線分
struct Segment
{
	Vector3 origin; //!< 始点
	Vector3 diff; //!< 終点への差分ベクトル
};
/// カプセル
struct Capsule
{
	Segment segment;
	float radius;
};