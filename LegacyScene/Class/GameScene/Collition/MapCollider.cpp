#include "MapCollider.h"
#include <cassert>

Vector2 MapCollider::GetHitChip(Vector3& position, float objSize, const std::vector<std::vector<uint32_t>>& map) {
	int mapWidth = static_cast<int>(map[0].size());
	int mapHeight = static_cast<int>(map.size());

	// 中心をfloatで計算
	float mapCenterX = (mapWidth - 1) / 2.0f;
	float mapCenterZ = (mapHeight - 1) / 2.0f;

	// ワールド座標→配列インデックス変換
	int objLeft = static_cast<int>(position.x - objSize / 2 + mapCenterX + 0.5f);
	int objRight = static_cast<int>(position.x + objSize / 2 + mapCenterX + 0.5f);
	int objTop = static_cast<int>(position.z - objSize / 2 + mapCenterZ + 0.5f);
	int objBottom = static_cast<int>(position.z + objSize / 2 + mapCenterZ + 0.5f);

	if (objLeft < 0 || objRight >= mapWidth || objTop < 0 || objBottom >= mapHeight) {
		return Vector2(-1.0f, -1.0f);
	}

	for (int z = objTop; z <= objBottom; ++z) {
		for (int x = objLeft; x <= objRight; ++x) {
			if (map[z][x] != 0) {
				return Vector2(static_cast<float>(x - mapCenterX), static_cast<float>(z - mapCenterZ));
			}
		}
	}

	return Vector2(-1.0f, -1.0f);
}

bool MapCollider::CheckMapCollition(Vector3& position, float objSize, const std::vector<std::vector<uint32_t>>& map) {
	int mapWidth = static_cast<int>(map[0].size());
	int mapHeight = static_cast<int>(map.size());

	// マップ中心座標
	int mapCenterX = mapWidth / 2;
	int mapCenterZ = mapHeight / 2;

	// ワールド座標→配列インデックス変換
	int objLeft = static_cast<int>((position.x - objSize / 2) + 0.5f) + mapCenterX;
	int objRight = static_cast<int>((position.x + objSize / 2) + 0.5f) + mapCenterX;
	int objTop = static_cast<int>((position.z - objSize / 2) + 0.5f) + mapCenterZ;
	int objBottom = static_cast<int>((position.z + objSize / 2) + 0.5f) + mapCenterZ;

	if (objLeft < 0 || objRight >= mapWidth || objTop < 0 || objBottom >= mapHeight) {
		assert(false && "Object is out of map bounds");
		return false;
	}

	for (int z = objTop; z <= objBottom; ++z) {
		for (int x = objLeft; x <= objRight; ++x) {
			if (map[z][x] != 0) {
				return true;
			}
		}
	}
	return false;
}

Vector3 MapCollider::GetReflectionDirection(Vector3& moveDir, Vector3& position, float objSize, const std::vector<std::vector<uint32_t>>& map) {
	Vector3 reflected = moveDir;

	// X方向の衝突判定
	Vector3 testPosX = position;
	testPosX.x += moveDir.x;
	if (CheckMapCollition(testPosX, objSize, map)) {
		reflected.x = -reflected.x;
	}

	// Z方向の衝突判定
	Vector3 testPosZ = position;
	testPosZ.z += moveDir.z;
	if (CheckMapCollition(testPosZ, objSize, map)) {
		reflected.z = -reflected.z;
	}

	// Y方向はマップチップで使わない前提（XZ平面のみ）
	return reflected;
}
