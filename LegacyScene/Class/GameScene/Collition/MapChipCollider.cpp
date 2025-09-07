#include "MapChipCollider.h"

Vector2 MapChipCollider::GetMapChipIndex(const Vector3& position) {
	float x = position.x / 2.0f;
	float y = position.z / 2.0f;
	return { x,y };
}

Vector3 MapChipCollider::GetMapChipPosition(const Vector2& index, float chipSize) {
	Vector3 pos;
	pos.x = index.x * chipSize;
	pos.y = 0.0f;
	pos.z = index.y * chipSize;
	return pos;
}

bool MapChipCollider::IsAABBCollision(const Vector2& aPos, float aWidth, float aHeight, const Vector2& bPos, float bWidth, float bHeight) {
	bool leftCheck = aPos.x < bPos.x + bWidth;
	bool rightCheck = aPos.x + aWidth > bPos.x;
	bool topCheck = aPos.y < bPos.y + bHeight;
	bool bottomCheck = aPos.y + aHeight > bPos.y;

	// すべての条件が成立した場合、当たり判定成立
	return leftCheck && rightCheck && topCheck && bottomCheck;
}
