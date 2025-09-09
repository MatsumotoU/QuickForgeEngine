#include "StageSelectBlocks.h"
#include "../Engine/Math/Transform.h"
#include "../Engine/Math/Vector/Vector4.h"
#include "../Engine/Particle/Particle.h"
#include "../LegacyScene/StageSelectScene.h"

void StageSelectBlocks::Initialize(const StageData &stageData, uint32_t currentStage, MapChipType mapChipType) {
	uint32_t size = MapChipField::kNumBlockHorizontal * MapChipField::kNumBlockVertical * StageSelectScene::kNumMapType;
	transforms_.resize(size);
	for (uint32_t i = 0; i < StageSelectScene::kNumMapType; ++i) {
		for (uint32_t j = 0; j < MapChipField::kNumBlockHorizontal; ++j) {
			for (uint32_t k = 0; k < MapChipField::kNumBlockVertical; ++k) {
				if (stageData.stageMapChipFields[i][currentStage].GetMapChipTypeByIndex(j, k) != mapChipType) {
					uint32_t index = i * MapChipField::kNumBlockHorizontal;
					index *= MapChipField::kNumBlockVertical;
					index += j * MapChipField::kNumBlockHorizontal + k;
					transforms_[index].scale = Vector3{ 0.0f, 0.0f, 0.0f };
					continue;
				}

				uint32_t index = i * MapChipField::kNumBlockHorizontal;
				index *= MapChipField::kNumBlockVertical;
				index += j * MapChipField::kNumBlockHorizontal + k;

				transforms_[index].translate = Vector3{
					static_cast<float>(j) - 3.5f,
					-static_cast<float>(StageSelectScene::kNumMapType - i) - 4.0f,
					static_cast<float>(MapChipField::kNumBlockVertical - k - 1) - 3.5f
				};
				transforms_[index].scale = Vector3{ 0.2f, 0.2f, 0.2f };
				transforms_[index].translate *= transforms_[index].scale;
			}
		}
	}
}

void StageSelectBlocks::Update() {
	particle_->Update(&transforms_);

	for (uint32_t i = 0; i < particle_->worldMatrixes_.size(); ++i) {
		particle_->worldMatrixes_[i] = particle_->worldMatrixes_[i] * parentWorldMatrix_;
	}
}

void StageSelectBlocks::Draw() {
	particle_->Draw(&colors_, camera_);
}