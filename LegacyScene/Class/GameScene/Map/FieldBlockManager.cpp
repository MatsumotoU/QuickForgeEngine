#include "FieldBlockManager.h"

void FieldBlockManager::Initialize(
	EngineCore* engineCore, Camera* camera, std::vector<std::vector<uint32_t>>& map, float width, float height) {

    for (size_t i = 0; i < map.size(); ++i) {
        for (size_t j = 0; j < map[i].size(); ++j) {
            if (map[i][j] != 0) {
                auto block = std::make_unique<Block>();
                block->Initialize(engineCore, camera);
                fieldBlocks_.push_back(std::move(block));
                width;
                height;
            }
        }
    }
}
