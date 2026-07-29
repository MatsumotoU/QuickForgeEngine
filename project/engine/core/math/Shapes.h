#pragma once
#include "Vector/Vector3.h"

namespace QFE::MATH {

	/// @brief 球
	struct Sphere
	{
		Vector3 center;
		float radius;
		int subdivision;
	};
	/// @brief 軸並行境界ボックス
	struct AABB
	{
		Vector3 center;
		Vector3 size;
	};
	/// @brief 任意方向境界ボックス
	struct OBB
	{
		Vector3 center;
		Vector3 orientations[3];
		Vector3 size;
	};
	/// @brief 平面
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
	/// 直緁E
	struct Line
	{
		Vector3 origin; // 始点
		Vector3 diff; // 終点への差刁Eクトル
	};
	/// 半直緁E
	struct Ray
	{
		Vector3 origin; //!< 始点
		Vector3 diff; //!< 終点への差刁Eクトル
	};
	/// 線E
	struct Segment
	{
		Vector3 origin; //!< 始点
		Vector3 diff; //!< 終点への差刁Eクトル
	};
	/// カプセル
	struct Capsule
	{
		Segment segment;
		float radius;
	};

}
