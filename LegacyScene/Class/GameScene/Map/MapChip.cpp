#include "MapChip.h"

void MapChip::Initialize(EngineCore* engineCore, Camera* camera) {
	camera_ = camera;
	for (uint32_t y = 0; y < kMapHeight; y++) {
		for (uint32_t x = 0; x < kMapWidth; x++) {
			blocks_[x + y * kMapWidth] = std::make_unique<Block>();
			blocks_[x + y * kMapWidth]->Initialize(engineCore, camera,BlockType::None);
			blocks_[x + y * kMapWidth]->transform_.translate.x = x * kBlockSize;
			blocks_[x + y * kMapWidth]->transform_.translate.y = 0.0f;
			blocks_[x + y * kMapWidth]->transform_.translate.z = y * kBlockSize;
		}
	}
	blocks_[0]->SetColor({ 1.0f,0.0f,0.0f,1.0f });

	uint32_t totalParticles = kMapWidth * kMapHeight;
	dirtBlock_.Initialize(engineCore, totalParticles);
	grassBlock_.Initialize(engineCore, totalParticles);
	stoneBlock_.Initialize(engineCore, totalParticles);

	dirtBlock_.LoadModel("Resources/Model/blocks/dirt", "dirt.obj", COORDINATESYSTEM_HAND_LEFT);
	grassBlock_.LoadModel("Resources/Model/blocks/grass", "grass.obj", COORDINATESYSTEM_HAND_LEFT);
	stoneBlock_.LoadModel("Resources/Model/blocks/stone", "stone.obj", COORDINATESYSTEM_HAND_LEFT);

	dirtBlockTransforms_.resize(totalParticles);
	dirtBlockColors_.resize(totalParticles, { 1.0f,1.0f,1.0f,1.0f });
	grassBlockTransforms_.resize(totalParticles);
	grassBlockColors_.resize(totalParticles, { 1.0f,1.0f,1.0f,1.0f });
	stoneBlockTransforms_.resize(totalParticles);
	stoneBlockColors_.resize(totalParticles, { 1.0f,1.0f,1.0f,1.0f });
}

void MapChip::Update() {
	ResetAllBlock();

	int index = 0;
	for (auto& block : blocks_) {
		if (!block->GetIsDraw()) {
			index++;
			continue;
		}

		switch (block->GetType())
		{
		case BlockType::Dirt:
			dirtBlockTransforms_[index] = block->transform_;
			dirtBlockColors_[index] = block->color_;
			break;
		case BlockType::Grass:
			grassBlockTransforms_[index] = block->transform_;
			grassBlockColors_[index] = block->color_;
			break;
		case BlockType::Stone:
			stoneBlockTransforms_[index] = block->transform_;
			stoneBlockColors_[index] = block->color_;
			break;
		default:
			break;
		}
		block->Update();

		index++;
	}
}

void MapChip::Draw() {
	dirtBlock_.Draw(&dirtBlockTransforms_, &dirtBlockColors_, camera_);
	grassBlock_.Draw(&grassBlockTransforms_, &grassBlockColors_, camera_);
	stoneBlock_.Draw(&stoneBlockTransforms_, &stoneBlockColors_, camera_);

}

void MapChip::SetMapPosition(const Vector3& position) {
	for (uint32_t y = 0; y < kMapHeight; y++) {
		for (uint32_t x = 0; x < kMapWidth; x++) {
			Transform transform;
			transform.translate.x = x * 1.0f + position.x;
			transform.translate.y = 0.0f + position.y;
			transform.translate.z = y * 1.0f + position.z;

			blocks_[x + y * kMapWidth]->SetTransform(transform);
		}
	}
}

void MapChip::SetMap(std::vector<std::vector<uint32_t>>& map) {
	for (uint32_t y = 0; y < kMapHeight; y++) {
		for (uint32_t x = 0; x < kMapWidth; x++) {
			if (map[y][x] == 3) {
				blocks_[x + y * kMapWidth]->BuildUpSpawn();
				blocks_[x + y * kMapWidth]->SetType(BlockType::Stone);
			}
			if (map[y][x] == 2) {
				blocks_[x + y * kMapWidth]->BuildUpSpawn();
				blocks_[x + y * kMapWidth]->SetType(BlockType::Grass);
			}
			if (map[y][x] == 1) {
				blocks_[x + y * kMapWidth]->BuildUpSpawn();
				blocks_[x + y * kMapWidth]->SetType(BlockType::Dirt);
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

void MapChip::ResetAllBlock() {
	for (Transform& transform : dirtBlockTransforms_) {
		transform.translate.y = 1000.0f;
	}
	for (Transform& transform : grassBlockTransforms_) {
		transform.translate.y = 1000.0f;
	}
	for (Transform& transform : stoneBlockTransforms_) {
		transform.translate.y = 1000.0f;
	}
}
