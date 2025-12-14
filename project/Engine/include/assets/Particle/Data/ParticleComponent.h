#pragma once
#include <cstdint>
#include <string>
#include "engine/include/core/Entity/Component/ComponentData.h"

class ParticleComponent final :public ComponentData {
public:
	std::string modelName;
	uint32_t maxParticleCount;

	uint32_t vartexBufferHandle;
	uint32_t particleGpuBufferHandle;
	uint32_t materialHandle;
	uint32_t textureHandle;

	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	std::string GetTypeName() const override { return "ParticleComponent"; }
};
