#pragma once
#include "Base/EngineCore.h"

class MapChipLoader final {
public:
	static std::vector<std::vector<uint32_t>> Load(const std::string& filePath);
};