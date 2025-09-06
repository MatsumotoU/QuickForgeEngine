#include "PredictionLine.h"
#include <cmath>
#include <set>
#include "../Collition/MapChipCollider.h"
#include "../Collition/MapReflection.h"

PredictionLine::PredictionLine() {
	isFullLine_ = false;
	isOutOfMap_ = false;
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

    // 反射したことのあるブロック座標を記録
    std::set<std::pair<int, int>> reflectedBlocks;

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

        for (int y = 0; y < static_cast<int>(wallMap.size()); y++) {
            for (int x = 0; x < static_cast<int>(wallMap[y].size()); x++) {
                // 反射済みブロックはスキップ
                if (reflectedBlocks.count({ x, y }) > 0) {
                    continue;
                }
                if (wallMap[y][x] != 0) {
                    Vector2 mapChipPos = { static_cast<float>(x) * kBlockSize - (kBlockSize * 0.5f), static_cast<float>(y) * kBlockSize - (kBlockSize * 0.5f) };
                    if (MapChipCollider::IsAABBCollision(playerPos, 1.0f, 1.0f, mapChipPos, kBlockSize, kBlockSize)) {
                        Vector2 blockCenter = mapChipPos + Vector2(kBlockSize * 0.5f, kBlockSize * 0.5f);
                        float distance = Vector2::Distance(playerCenter, blockCenter);
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

        if (isCollided) {
            direction = MapReflection::ReflectDirection(direction, playerPos, 1.0f, 1.0f, nearestMapChipPos, kBlockSize, kBlockSize);
            direction = direction.Normalize();
            currentPos = nextPos;
            // 反射したブロック座標を記録
            reflectedBlocks.insert({ nearestX, nearestY });
            continue;
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
	if (linePoints_.size() < 2) return;
	GraphRenderer* graphRenderer = engineCore->GetGraphRenderer();
	for (size_t i = 0; i < linePoints_.size() - 1; ++i) {
		Vector3 startPos = { linePoints_[i].x, 1.0f, linePoints_[i].y };
		Vector3 endPos = { linePoints_[i + 1].x, 1.0f, linePoints_[i + 1].y };

		if (isOutOfMap_) {
            graphRenderer->DrawLine(startPos, endPos, { 1.0f, 0.0f, 0.0f, 1.0f });
		} else {
            graphRenderer->DrawLine(startPos, endPos, { 0.0f, 1.0f, 0.0f, 1.0f });
			
		}
	}
}

std::vector<Vector2>& PredictionLine::GetLinePoints() {
	return linePoints_;
}
