#pragma once
#include "Vector/Vector3.h"

/// 琁E��E
struct Sphere
{
	Vector3 center;
	float radius;
	int subdivision;
};
/// 軸並行墁E��ボックス
struct AABB
{
	Vector3 center;
	Vector3 size;
};
/// 有向墁E��ボックス
struct OBB
{
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};
/// 板
struct Plane
{
	Vector3 normal; // 法緁E
	float distance; // 距離

	float DistanceToPoint(const Vector3& point);
};
/// 三角形
struct Triangle {
	Vector3 vertices[3];
};
/// 直緁E
struct Line
{
	Vector3 origin; // 始点
	Vector3 diff; // 終点への差刁E�Eクトル
};
/// 半直緁E
struct Ray
{
	Vector3 origin; //!< 始点
	Vector3 diff; //!< 終点への差刁E�Eクトル
};
/// 線�E
struct Segment
{
	Vector3 origin; //!< 始点
	Vector3 diff; //!< 終点への差刁E�Eクトル
};
/// カプセル
struct Capsule
{
	Segment segment;
	float radius;
};
