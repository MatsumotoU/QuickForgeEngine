#include "LandingPoint.h"

//void LandingPoint::Init() {
//	isOutOfMap_ = false;
//}
//
//void LandingPoint::Scan(const Vector3& jumpPos, const std::vector<std::vector<uint32_t>>& wallMap, const std::vector<std::vector<uint32_t>>& floorMap, float kBlockSize) {
//	isOutOfMap_ = false;
//
//	// ジャンプ方向（XZ平面で進む）
//	Vector2 jumpDir(jumpPos.x, jumpPos.z);
//	if (jumpDir.Length() > 0.0f) {
//		jumpDir = jumpDir.Normalize();
//	} else {
//		jumpDir = Vector2(0.0f, 1.0f); // デフォルト上方向
//	}
//
//	Vector2 currentPos(jumpPos.x, jumpPos.z);
//	const int maxStep = 20; // 予測する最大ステップ数
//
//	for (int step = 0; step < maxStep; ++step) {
//		currentPos += jumpDir * kBlockSize;
//
//		int mapX = static_cast<int>(currentPos.x / kBlockSize);
//		int mapY = static_cast<int>(currentPos.y / kBlockSize);
//
//		// マップ範囲外なら終了
//		if (mapX < 0 || mapX >= static_cast<int>(floorMap[0].size()) ||
//			mapY < 0 || mapY >= static_cast<int>(floorMap.size())) {
//			isOutOfMap_ = true;
//			break;
//		}
//
//		// 床があれば着地
//		if (floorMap[mapY][mapX] != 0) {
//			landingPoints_=currentPos;
//			break; // 最初の着地点のみ記録
//		}
//	}
//}

void LandingPoint::Init() {
	isOutOfMap_ = false;
	landingPoints_.clear();

}

void LandingPoint::Scan(const Vector3& jumpPos, const Vector2 jumpDir, const std::vector<std::vector<uint32_t>>& floorMap, float kBlockSize) {
	landingPoints_.clear();
	landingPoints_.push_back(jumpPos);
	isOutOfMap_ = false;

	Vector3 currentPos = jumpPos;
	Vector3 velocity(jumpDir.Normalize().x * 2.0f, 15.0f, jumpDir.Normalize().y * 2.0f); // 初速のY成分
	Vector2 currentVelocity = { velocity.x, velocity.z };
	while (currentVelocity.Length() > 0.05f)
	{
		currentPos += velocity;
		velocity = velocity * 0.98f * 0.16f;
		if (velocity.y > -5.0f) {
			velocity.y -= 9.81f * 0.16f;
		}
		if (currentPos.y <= 1.0f) {
			currentPos.y = 1.0f;
			velocity.y = 0.0f;
			break;
		}
		currentVelocity = { velocity.x, velocity.z };
		landingPoints_.push_back(currentPos);
	}

	int mapX = static_cast<int>(currentPos.x / kBlockSize);
	int mapY = static_cast<int>(currentPos.z / kBlockSize);
	// マップ範囲外なら終了
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
		Vector3 startPos = { landingPoints_[i].x, 1.0f, landingPoints_[i].z };
		Vector3 endPos = { landingPoints_[i + 1].x, 1.0f, landingPoints_[i + 1].z };

		if (isOutOfMap_) {
			graphRenderer->DrawLine(startPos, endPos, { 1.0f, 0.0f, 0.0f, 1.0f });
		} else {
			graphRenderer->DrawLine(startPos, endPos, { 1.0f, 1.0f, 0.0f, 1.0f });
		}
	}
}