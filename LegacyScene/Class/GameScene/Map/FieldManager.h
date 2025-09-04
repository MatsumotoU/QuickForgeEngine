#pragma once
#include "Base/EngineCore.h"
#include <vector>

class FieldManager final{
public:
	void LoadMapChip(const std::string& filePath);
	uint32_t GetMapChipValue(int row, int col) const;

private:
	std::vector<std::vector<uint32_t>> mapChipData_;
};