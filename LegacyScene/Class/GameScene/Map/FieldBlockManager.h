#pragma once
#include "Base/EngineCore.h"
#include "../Block.h"

class FieldBlockManager final {
public:
	void Initialize(EngineCore* engineCore,Camera* camera,std::vector<std::vector<uint32_t>>& map,float width,float height);

private:
	std::vector<std::unique_ptr<Block>> fieldBlocks_;
};