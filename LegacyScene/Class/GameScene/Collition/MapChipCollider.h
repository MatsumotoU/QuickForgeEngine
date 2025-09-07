#pragma once
#include <vector>
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector2.h"

class MapChipCollider {
public:
	static Vector2 GetMapChipIndex(const Vector3& position);
	static Vector3 GetMapChipPosition(const Vector2& index,float chipSize);
	static bool IsAABBCollision(
		const Vector2& aPos, float aWidth, float aHeight,
		const Vector2& bPos, float bWidth, float bHeight
	);
};