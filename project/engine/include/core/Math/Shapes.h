#pragma once
#include "Vector/Vector3.h"

/// 逅・ｽ・
struct Sphere
{
	Vector3 center;
	float radius;
	int subdivision;
};
/// 霆ｸ荳ｦ陦悟｢・阜繝懊ャ繧ｯ繧ｹ
struct AABB
{
	Vector3 min;
	Vector3 max;
};
/// 譛牙髄蠅・阜繝懊ャ繧ｯ繧ｹ
struct OBB
{
	Vector3 center;
	Vector3 orientations[3];
	Vector3 size;
};
/// 譚ｿ
struct Plane
{
	Vector3 normal; // 豕慕ｷ・
	float distance; // 霍晞屬

	float DistanceToPoint(const Vector3& point);
};
/// 荳芽ｧ貞ｽ｢
struct Triangle {
	Vector3 vertices[3];
};
/// 逶ｴ邱・
struct Line
{
	Vector3 origin; // 蟋狗せ
	Vector3 diff; // 邨らせ縺ｸ縺ｮ蟾ｮ蛻・・繧ｯ繝医Ν
};
/// 蜊顔峩邱・
struct Ray
{
	Vector3 origin; //!< 蟋狗せ
	Vector3 diff; //!< 邨らせ縺ｸ縺ｮ蟾ｮ蛻・・繧ｯ繝医Ν
};
/// 邱壼・
struct Segment
{
	Vector3 origin; //!< 蟋狗せ
	Vector3 diff; //!< 邨らせ縺ｸ縺ｮ蟾ｮ蛻・・繧ｯ繝医Ν
};
/// 繧ｫ繝励そ繝ｫ
struct Capsule
{
	Segment segment;
	float radius;
};
