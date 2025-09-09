#include "LandingPoint.h"
#include "../Collition/MapChipCollider.h"

void LandingPoint::Init() {
	isOutOfMap_ = false;
	landingPoints_.clear();

}

void LandingPoint::Scan(
    const Vector3& jumpPos,
    const std::vector<IntVector2>& mapChipIndices,
    const std::vector<std::vector<uint32_t>>& floorMap,
    float kBlockSize
) {
    landingPoints_.clear();
    isOutOfMap_ = false;

    // プレイヤー座標（JumpingUpdateと同じ計算式）
    Vector2 playerPos = { jumpPos.x - 0.5f, jumpPos.z - 0.5f };
    Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);

    // 合成法線計算
    Vector2 totalNormal(0.0f, 0.0f);
    bool isCollided = false;
    for (const auto& mapchip : mapChipIndices) {
        // IntVector2型のmapchip.x, mapchip.yをfloatに変換
        Vector2 mapChipPos = { static_cast<float>(mapchip.x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(mapchip.y) * kBlockSize - (kBlockSize * 0.5f) };
        if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
            Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
            Vector2 jumpDir = (playerCenter - blockCenter).Normalize();
            totalNormal += jumpDir;
            isCollided = true;
        }
    }

    // 衝突がなければ描画しない
    if (!isCollided) {
        landingPoints_.clear();
        isOutOfMap_ = false;
        return;
    }

    // ジャンプ方向決定
    Vector2 jumpDir;
    if (totalNormal.Length() > 0.01f) {
        jumpDir = totalNormal.Normalize();
    } else {
        jumpDir = Vector2(0.0f, 1.0f); // デフォルトで上方向
    }

    // ジャンプ軌道予測
    landingPoints_.push_back(jumpPos);
    Vector3 currentPos = jumpPos;
    Vector3 velocity(jumpDir.x * 2.0f, 15.0f, jumpDir.y * 2.0f); // 初速
    Vector2 currentVelocity = { velocity.x, velocity.z };
    while (currentVelocity.Length() > 0.05f) {
        currentPos += velocity;
        velocity = velocity * (0.98f * 0.16f);
        if (velocity.y > -5.0f) {
            velocity.y -= 9.81f * 0.16f;
        }
        if (currentPos.y <= 1.0f) {
            currentPos.y = 1.0f;
            velocity.y = 0.0f;
            velocity.x = 0.0f;
            velocity.z = 0.0f;
            break;
        }
        currentVelocity = { velocity.x, velocity.z };
        landingPoints_.push_back(currentPos);
    }

    int mapX = static_cast<int>(currentPos.x / kBlockSize);
    int mapY = static_cast<int>(currentPos.z / kBlockSize);
    if (mapX < 0 || mapX >= static_cast<int>(floorMap[0].size()) ||
        mapY < 0 || mapY >= static_cast<int>(floorMap.size())) {
        isOutOfMap_ = true;
        return;
    }
    if (floorMap[mapY][mapX] == 0) {
        isOutOfMap_ = true;
        return;
    }
}

void LandingPoint::Draw(EngineCore* engineCore) {
	if (landingPoints_.size() < 2) return;
	GraphRenderer* graphRenderer = engineCore->GetGraphRenderer();
	for (size_t i = 0; i < landingPoints_.size() - 1; ++i) {
		//Vector3 startPos = { landingPoints_[i].x, 1.0f, landingPoints_[i].z };
		//Vector3 endPos = { landingPoints_[i + 1].x, 1.0f, landingPoints_[i + 1].z };
		Vector3 startPos = landingPoints_[i];
		Vector3 endPos = landingPoints_[i + 1];

		if (isOutOfMap_) {
			graphRenderer->DrawLine(startPos, endPos, { 1.0f, 0.0f, 0.0f, 1.0f });
		} else {
			graphRenderer->DrawLine(startPos, endPos, { 1.0f, 1.0f, 0.0f, 1.0f });
		}
	}
}