#include "MapChip.h"
#include <map>

namespace {
	std::map<std::string, MapChipType> mapChipTable = {
		{"0", MapChipType::kBlank},
		{"1", MapChipType::kBlock},
	};
}

void MapChip::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;

	blocks_.clear();
	LoadMapChipCsv("Resources/blocks.csv");

	for (int y = 0; y < kNumBlockVertical; y++) {
		for (int x = 0; x < kNumBlockHorizontal; x++) {

			if (mapChipData_.data[y][x] == MapChipType::kBlock) {
				blocks_.push_back(std::move(Block()));
				blocks_.back().Initialize(engineCore_);
				blocks_.back().transform_.translate = GetMapChipPositionByIndex(x, y);
				blocks_.back().Update();
			}
		}
	}
}

void MapChip::Draw(Camera* camera) {
	for (int i = 0; i < blocks_.size(); i++) {
		blocks_[i].Draw(camera);
	}
}

void MapChip::ResetMapChipData() {
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVertical);
	for (std::vector<MapChipType>& mapChipLine : mapChipData_.data) {
		mapChipLine.resize(kNumBlockHorizontal);
	}
}

void MapChip::LoadMapChipCsv(const std::string& filePath) {
	ResetMapChipData();
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	std::stringstream mapChipCsv;
	mapChipCsv << file.rdbuf();
	file.close();

	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		std::string line;
		std::getline(mapChipCsv, line);
		std::istringstream lineStream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			std::getline(lineStream, word, ',');
			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChip::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex < 0 || kNumBlockVertical - 1 < yIndex) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex];
}

Vector3 MapChip::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {
	return Vector3(kBlockWidth * 0.5f + kBlockWidth * static_cast<float>(xIndex), kBlockHeight * 0.5f + kBlockHeight * (kNumBlockVertical - 1.0f - static_cast<float>(yIndex)), 0.0f);
}