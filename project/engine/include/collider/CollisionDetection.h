#pragma once
#include "engine/include/core/Math/Shapes.h"

namespace QFE::COLLIDER {
	/// @brief 衝突判定関数
	bool isCollision(const Sphere& sphere1, const Sphere& sphere2);
	bool isCollision(const AABB& aabb1, const AABB& aabb2);
	bool isCollision(const Sphere& sphere, const AABB& aabb);
}
