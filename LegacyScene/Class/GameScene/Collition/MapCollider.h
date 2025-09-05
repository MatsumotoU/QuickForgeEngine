#pragma once
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector2.h"
#include <vector>

class MapCollider {
public:
	static Vector2 GetHitChip(Vector3& position, float objSize, const std::vector<std::vector<uint32_t>>& map);
	static bool CheckMapCollition(Vector3& position, float objSize, const std::vector<std::vector<uint32_t>>& map);
	static Vector3 GetReflectionDirection(Vector3& moveDir, Vector3& position, float objSize, const std::vector<std::vector<uint32_t>>& map);
};
