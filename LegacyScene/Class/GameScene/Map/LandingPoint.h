#pragma once
#include "Base/EngineCore.h"
#include "Math/Vector/IntVector2.h"
#include <vector>

class LandingPoint final {
public:
	LandingPoint() = default;
	~LandingPoint() = default;

	void Init();
	void Scan(const Vector3& jumpPos,
		const std::vector<IntVector2>& mapChipIndices,
		const std::vector<std::vector<uint32_t>>& floorMap,
		float kBlockSize);
	void Draw(EngineCore* engineCore);

	std::vector<Vector3> GetLandingPoints() const { return landingPoints_; }

private:
	std::vector<Vector3> landingPoints_;
	bool isOutOfMap_;
};