#pragma once
#include "Base/EngineCore.h"

class LandingPoint final {
public:
	LandingPoint() = default;
	~LandingPoint() = default;

	/*void Init();
	void Scan(const Vector3& jumpPos,const std::vector<std::vector<uint32_t>>& wallMap, const std::vector<std::vector<uint32_t>>& floorMap, float kBlockSize);
	void Draw(EngineCore* engineCore);*/
private:
	Vector2 landingPoints_;
	bool isOutOfMap_;
};