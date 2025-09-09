#pragma once
#include "Base/EngineCore.h"
#include <vector>
#include "Math/Vector/Vector2.h"

class PredictionLine final {
public:
	PredictionLine();
	~PredictionLine() = default;

	void Init();
	void Scan(const Vector3& startPos, const Vector2& moveDir,int numTiles, const std::vector<std::vector<uint32_t>>& wallMap, float kBlockSize);
	void Draw(EngineCore* engineCore);

	std::vector<Vector2>& GetLinePoints();

private:
	std::vector<Vector2> linePoints_;
	bool isFullLine_;
	bool isOutOfMap_;
	float timer_;
};