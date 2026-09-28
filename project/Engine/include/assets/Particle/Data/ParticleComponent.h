#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/Math/Vector/Vector4.h"
#include "engine/include/core/Entity/Component/ComponentData.h"

class AssetManager;
struct ParticleForGPU;

struct ParticleRuntimeData {
	Vector3 position{ 0.0f, 0.0f, 0.0f };
	Vector3 velocity{ 0.0f, 0.0f, 0.0f };
	Vector3 rotation{ 0.0f, 0.0f, 0.0f };
	Vector3 angularVelocity{ 0.0f, 0.0f, 0.0f };
	float age = 0.0f;
	float lifetime = 0.0f;
	float startScale = 0.0f;
	bool isActive = false;
};

class ParticleComponent final :public ComponentData {
public:
	std::string modelName;
	uint32_t maxParticleCount = 1;
	float minLifetime = 0.25f;
	float maxLifetime = 0.75f;
	float minSpeed = 1.0f;
	float maxSpeed = 4.0f;
	float startScale = 0.2f;
	float endScale = 0.0f;
	float scaleVariation = 0.25f;
	float gravity = 0.0f;
	float directionSpread = 0.8f;
	Vector4 startColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector4 endColor{ 1.0f, 1.0f, 1.0f, 0.0f };

	uint32_t vartexBufferHandle = 0;
	uint32_t particleGpuBufferHandle = 0;
	uint32_t materialHandle = 0;
	uint32_t textureHandle = 0;
	uint32_t nextParticleIndex = 0;
	std::vector<ParticleRuntimeData> particles;

	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	void InitializeResources(AssetManager& assetManager);
	void EmitBurst(const Vector3& position, uint32_t count, const Vector3& direction);
	void Update(float deltaTime, ParticleForGPU* gpuData);
	std::string GetTypeName() const override { return "ParticleComponent"; }
};
