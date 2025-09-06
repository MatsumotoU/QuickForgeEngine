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
