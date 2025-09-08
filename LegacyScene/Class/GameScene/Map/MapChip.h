#pragma once
#include "Base/EngineCore.h"
#include "../Block.h"
#include "Particle/Particle.h"

static inline const uint32_t kMapWidth = 8;
static inline const uint32_t kMapHeight = 8;

class MapChip final {
public:
	void Initialize(EngineCore* engineCore,Camera* camera);
	void Update();
	void Draw();

	void SetMapPosition(const Vector3& position);
	void SetMap(std::vector<std::vector<uint32_t>>& map);
	void SetChipColor(uint32_t x, uint32_t y,const Vector4& color);
	void ResetChipColor();
	void ResetAllBlock();

private:
	Camera* camera_;

	std::array<std::unique_ptr<Block>, kMapWidth * kMapHeight> blocks_;
	Particle dirtBlock_;
	std::vector<Transform> dirtBlockTransforms_;
	std::vector<Vector4> dirtBlockColors_;
	Particle grassBlock_;
	std::vector<Transform> grassBlockTransforms_;
	std::vector<Vector4> grassBlockColors_;
	Particle stoneBlock_;
	std::vector<Transform> stoneBlockTransforms_;
	std::vector<Vector4> stoneBlockColors_;
};