#pragma once
#include "Math/Vector/Vector2.h"

class MapReflection {
public:
	static void ReflectPlayer(Vector2& velocity, const Vector2& playerPos, float playerWidth, float playerHeight,
		const Vector2& blockPos, float blockWidth, float blockHeight);

};