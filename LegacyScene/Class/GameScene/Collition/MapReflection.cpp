#include "MapReflection.h"
#include <algorithm>

void MapReflection::ReflectPlayer(Vector2& velocity, const Vector2& playerPos, float playerWidth, float playerHeight, const Vector2& blockPos, float blockWidth, float blockHeight) {
    // プレイヤーとブロックの各辺の重なり量を計算
    float overlapLeft = (playerPos.x + playerWidth) - blockPos.x;
    float overlapRight = (blockPos.x + blockWidth) - playerPos.x;
    float overlapTop = (playerPos.y + playerHeight) - blockPos.y;
    float overlapBottom = (blockPos.y + blockHeight) - playerPos.y;

    float minOverlapX = std::min(overlapLeft, overlapRight);
    float minOverlapY = std::min(overlapTop, overlapBottom);

    // 最小オーバーラップ方向で反射
    if (minOverlapX < minOverlapY) {
        // 横面で反射
        velocity.x = -velocity.x;
    } else {
        // 上底面で反射
        velocity.y = -velocity.y;
    }
}
