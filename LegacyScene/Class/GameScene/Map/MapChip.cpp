#include "MapChip.h"

void MapChip::Initialize(EngineCore* engineCore, Camera* camera) {
	for (uint32_t y = 0; y < kMapHeight; y++) {
		for (uint32_t x = 0; x < kMapWidth; x++) {
			blocks_[x + y * kMapWidth] = std::make_unique<Block>();
			blocks_[x + y * kMapWidth]->Initialize(engineCore, camera);
			blocks_[x + y * kMapWidth]->GetTransform().translate.x = x * kBlockSize;
			blocks_[x + y * kMapWidth]->GetTransform().translate.y = 0.0f;
			blocks_[x + y * kMapWidth]->GetTransform().translate.z = y * kBlockSize;
		}
	}
	blocks_[0]->SetColor({ 1.0f,0.0f,0.0f,1.0f });
}

void MapChip::Update() {
	for (auto& block : blocks_) {
		block->Update();
	}
}

void MapChip::Draw() {
	for (auto& block : blocks_) {
		block->Draw();
	}
}

void MapChip::SetMapPosition(const Vector3& position) {
	for (uint32_t y = 0; y < kMapHeight; y++) {
		for (uint32_t x = 0; x < kMapWidth; x++) {
			blocks_[x + y * kMapWidth]->GetTransform().translate.x = x * 1.0f + position.x;
			blocks_[x + y * kMapWidth]->GetTransform().translate.y = 0.0f + position.y;
			blocks_[x + y * kMapWidth]->GetTransform().translate.z = y * 1.0f + position.z;
		}
	}
}

void MapChip::SetMap(std::vector<std::vector<uint32_t>>& map) {
	for (uint32_t y = 0; y < kMapHeight; y++) {
		for (uint32_t x = 0; x < kMapWidth; x++) {
			if (map[y][x] == 1) {
				blocks_[x + y * kMapWidth]->SetIsDraw(true);
			}
			if (map[y][x] == 0) {
				blocks_[x + y * kMapWidth]->SetIsDraw(false);
			}
		}
	}
}

void MapChip::SetChipColor(uint32_t x, uint32_t y, const Vector4& color) {
	if (x < kMapWidth && y < kMapHeight) {
		blocks_[x + y * kMapWidth]->SetColor(color);
	}
}

void MapChip::ResetChipColor() {
	for (auto& block : blocks_) {
		block->SetColor({ 1.0f,1.0f,1.0f,1.0f });
	}
}
