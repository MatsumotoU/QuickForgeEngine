#include "PredictionLine.h"
#include <cmath>
#include <set>
#include "../Collition/MapChipCollider.h"
#include "../Collition/MapReflection.h"

PredictionLine::PredictionLine() {
	isFullLine_ = false;
	isOutOfMap_ = false;
    timer_ = 0.0f;
}

void PredictionLine::Init() {
	linePoints_.clear();
	isFullLine_ = false;
    isOutOfMap_ = false;
	
}

void PredictionLine::Scan(const Vector3& startPos, const Vector2& moveDir, int numTiles, const std::vector<std::vector<uint32_t>>& wallMap, float kBlockSize) {
    linePoints_.clear();
    Vector2 currentPos = { startPos.x, startPos.z };
    Vector2 direction = moveDir.Normalize();
    float stepSize = 1.0f / static_cast<float>(numTiles);
    float totalDistance = 0.0f;

    bool reflectedThisFrame = false;
    std::map<std::pair<int, int>, int> reflectCountMap; // 座標ごとの反射回数

    for (int i = 0; i < numTiles * 30; ++i) {
        linePoints_.push_back(currentPos);
        Vector2 nextPos = currentPos + direction * stepSize;
        totalDistance += stepSize;

        float minDistance = std::numeric_limits<float>::max();
        int nearestX = -1;
        int nearestY = -1;
        Vector2 nearestMapChipPos;
        bool isCollided = false;

        Vector2 playerPos = nextPos - Vector2(0.5f, 0.5f);
        Vector2 playerCenter = playerPos + Vector2(0.5f, 0.5f);

        // 最も近いブロックのみで反射
        for (int y = 0; y < static_cast<int>(wallMap.size()); y++) {
            for (int x = 0; x < static_cast<int>(wallMap[y].size()); x++) {
                if (wallMap[y][x] != 0) {
                    // 反射回数制限
                    auto key = std::make_pair(x, y);
                    int maxReflect = (wallMap[y][x] == 3) ? 2 : 1;
                    if (reflectCountMap[key] >= maxReflect) continue;

                    Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
                    Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
                    float distance = Vector2::Distance(playerCenter, blockCenter);
                    if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
                        if (distance < minDistance) {
                            minDistance = distance;
                            nearestX = x;
                            nearestY = y;
                            nearestMapChipPos = mapChipPos;
                            isCollided = true;
                        }
                    }
                }
            }
        }

        if (isCollided && !reflectedThisFrame) {
            // 反射
            direction = MapReflection::ReflectDirection(direction, playerPos, 1.0f, 1.0f, nearestMapChipPos, kBlockSize, kBlockSize);
            direction = direction.Normalize();
            reflectedThisFrame = true;

            // 反射回数を記録
            auto key = std::make_pair(nearestX, nearestY);
            reflectCountMap[key]++;

            // めり込み防止: 線分座標をブロック外へ押し戻す
            Vector2 reflectDir = (playerCenter - (nearestMapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f))).Normalize();
            nextPos.x += reflectDir.x * 0.1f;
            nextPos.y += reflectDir.y * 0.1f;
        } else {
            reflectedThisFrame = false;
        }

        int mapX = static_cast<int>(std::floor(nextPos.x / kBlockSize));
        int mapY = static_cast<int>(std::floor(nextPos.y / kBlockSize));
        if (mapX < 0 || mapX >= static_cast<int>(wallMap[0].size()) ||
            mapY < 0 || mapY >= static_cast<int>(wallMap.size())) {
            isOutOfMap_ = true;
        }

        if (totalDistance >= static_cast<float>(numTiles)) {
            isFullLine_ = true;
            break;
        }

        currentPos = nextPos;
    }
}

void PredictionLine::Draw(EngineCore* engineCore) {
	timer_ += engineCore->GetDeltaTime(); 
	if (linePoints_.size() < 2) return;
	GraphRenderer* graphRenderer = engineCore->GetGraphRenderer();
    for (size_t i = 0; i < linePoints_.size() - 1; ++i) {
        Vector3 startPos = { linePoints_[i].x, 1.0f, linePoints_[i].y };
        Vector3 endPos = { linePoints_[i + 1].x, 1.0f, linePoints_[i + 1].y };

        // 色の指定
        Vector4 targetColor = isOutOfMap_ ? Vector4(1.0f, 0.0f, 0.0f, 1.0f) : Vector4(0.0f, 1.0f, 0.0f, 1.0f);
        Vector4 whiteColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

        // 波状補間率の計算
        float t = static_cast<float>(i) / static_cast<float>(linePoints_.size() - 1);
        float wave = 0.5f * (1.0f + std::sin(-timer_ * 10.0f + t * 6.28f)); // timer_でアニメーション

        // 色の補間
        Vector4 color = whiteColor * (1.0f - wave) + targetColor * wave;

        graphRenderer->DrawLine(startPos, endPos, color);
    }
}

std::vector<Vector2>& PredictionLine::GetLinePoints() {
	return linePoints_;
}
