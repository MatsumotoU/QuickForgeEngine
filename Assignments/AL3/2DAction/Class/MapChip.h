#pragma once
#include "Block.h"

static inline const float kBlockWidth = 2.0f;
static inline const float kBlockHeight = 2.0f;

static inline const uint32_t kNumBlockVertical = 20;
static inline const uint32_t kNumBlockHorizontal = 100;

enum class MapChipType {
	kBlank,
	kBlock,
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChip {
public:
	void Initialize(EngineCore* engineCore);
	void Draw(Camera* camera);

public:
	void ResetMapChipData();
	void LoadMapChipCsv(const std::string& filePath);
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

private:
	EngineCore* engineCore_;
	MapChipData mapChipData_;
	std::vector<Block> blocks_;
};