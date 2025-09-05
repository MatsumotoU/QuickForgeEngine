#include "MapChipLoader.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>

std::vector<std::vector<uint32_t>> MapChipLoader::Load(const std::string& filePath) {
    std::vector<std::vector<uint32_t>> mapChips;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        assert(false && "Failed to open map chip file.");
        return mapChips;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<uint32_t> row;
        while (std::getline(ss, cell, ',')) {
            try {
                uint32_t value = static_cast<uint32_t>(std::stoul(cell));
                row.push_back(value);
            }
            catch (const std::exception&) {
                // 数値変換に失敗した場合はスキップ
                continue;
            }
        }
        if (!row.empty()) {
            mapChips.push_back(row);
        }
    }

    return mapChips;
}
