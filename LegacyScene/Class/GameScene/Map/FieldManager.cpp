#include "FieldManager.h"
#include "MapChipLoader.h"
#include <cassert>

void FieldManager::LoadMapChip(const std::string& filePath) {
	mapChipData_ = MapChipLoader::Load(filePath);
}

uint32_t FieldManager::GetMapChipValue(int row, int col) const {
	assert(row >= 0 && row < static_cast<int>(mapChipData_.size()) && col >= 0 && col < static_cast<int>(mapChipData_[0].size()) && "Index out of bounds");
	return mapChipData_[row][col];
}
