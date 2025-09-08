#include "StageSelectBlocks.h"
#include "../Engine/Math/Transform.h"
#include "../Engine/Math/Vector/Vector4.h"
#include "../Engine/Particle/Particle.h"

void BaseStageSelectBlocks::Initialize() {
	for (uint32_t i = 0; i < 8; ++i) {
		for (uint32_t j = 0; j < 8; ++j) {
			Transform transform;
			transform.translate = Vector3{ static_cast<float>(i), -1.0f, static_cast<float>(j) };
			transform.scale = Vector3{ 0.5f, 0.5f, 0.5f };
			transform.translate *= transform.scale;
			transforms_.emplace_back(transform);
		}
	}
}

void BaseStageSelectBlocks::Draw() {
	particle_->Draw(&colors_, camera_);
}