#pragma once
#include "Base/EngineCore.h"
#include "../Block.h"

static inline const uint32_t kMapWidth = 8;
static inline const uint32_t kMapHeight = 8;

class MapChip final {
public:
	void Initialize(EngineCore* engineCore,Camera* camera);
	void Update();
	void Draw();

	void SetMapPosition(const Vector3& position);
	void SetMap(std::vector<std::vector<uint32_t>>& map);

private:
	std::array<std::unique_ptr<Block>, kMapWidth * kMapHeight> blocks_;
};