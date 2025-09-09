#include "StageSelectBlocks.h"
#include "../Engine/Math/Vector/Vector4.h"
#include "../Engine/Particle/Particle.h"
#include "../LegacyScene/StageSelectScene.h"
#include <numbers>

void StageSelectBlocks::Initialize(const StageData &stageData, uint32_t currentStage, MapChipType mapChipType) {
	uint32_t size = MapChipField::kNumBlockHorizontal * MapChipField::kNumBlockVertical * StageSelectScene::kNumMapType;
	transforms_.resize(size);
	for (uint32_t i = 0; i < StageSelectScene::kNumMapType; ++i) {
		for (uint32_t j = 0; j < MapChipField::kNumBlockHorizontal; ++j) {
			for (uint32_t k = 0; k < MapChipField::kNumBlockVertical; ++k) {
				uint32_t index = i * MapChipField::kNumBlockHorizontal;
				index *= MapChipField::kNumBlockVertical;
				index += j * MapChipField::kNumBlockHorizontal + k;

				if (stageData.stageMapChipFields[i][currentStage].GetMapChipTypeByIndex(j, k) != mapChipType) {
					transforms_[index].scale = Vector3{ 0.0f, 0.0f, 0.0f };
					continue;
				}

				transforms_[index].scale = Vector3{ 1.0f, 1.0f, 1.0f };
				transforms_[index].translate = Vector3{
					static_cast<float>(j) - 3.5f,
					-static_cast<float>(StageSelectScene::kNumMapType - i) - 3.0f,
					static_cast<float>(MapChipField::kNumBlockVertical - k - 1) - 3.5f
				};
			}
		}
	}
	anchorTransform_.scale = Vector3{ 0.2f, 0.2f, 0.2f };
}

void StageSelectBlocks::Update() {
	particle_->Update(&transforms_);
	anchorTransform_.rotate.y += std::numbers::pi_v<float> *2.0f / (60.0f * kRotateTime);
	anchorTransform_.rotate.y = std::fmodf(anchorTransform_.rotate.y, 2.0f * std::numbers::pi_v<float>);
	anchorWorldMatrix_ = Matrix4x4::MakeAffineMatrix(anchorTransform_.scale, anchorTransform_.rotate, anchorTransform_.translate);
	anchorWorldMatrix_ = anchorWorldMatrix_ * parentWorldMatrix_;

	for (uint32_t i = 0; i < particle_->worldMatrixes_.size(); ++i) {
		particle_->worldMatrixes_[i] = particle_->worldMatrixes_[i] * anchorWorldMatrix_;
	}
}

void StageSelectBlocks::Draw() {
	particle_->Draw(&colors_, camera_);
}