#include "engine/include/assets/Particle/Data/ParticleComponent.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/graphic/GpuBufferPool/GpuBufferPool.h"
#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

// Windows.h may define min/max macros that expand inside std::min/std::max.
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <algorithm>
#include <cmath>
#include <random>

namespace {
	constexpr uint32_t kMaxParticleCapacity = 4096;
	constexpr float kMinDirectionLengthSquared = 0.0001f;

	bool ReadColor(const nlohmann::json& json, Vector4& color) {
		if (!json.is_array() || json.size() < 4) {
			return false;
		}
		color = Vector4{
			json.at(0).get<float>(),
			json.at(1).get<float>(),
			json.at(2).get<float>(),
			json.at(3).get<float>() };
		return true;
	}
}

nlohmann::json ParticleComponent::Serialize() const {
	nlohmann::json json;
	json["modelName"] = modelName;
	json["maxParticleCount"] = maxParticleCount;
	json["minLifetime"] = minLifetime;
	json["maxLifetime"] = maxLifetime;
	json["minSpeed"] = minSpeed;
	json["maxSpeed"] = maxSpeed;
	json["startScale"] = startScale;
	json["endScale"] = endScale;
	json["scaleVariation"] = scaleVariation;
	json["gravity"] = gravity;
	json["directionSpread"] = directionSpread;
	json["startColor"] = { startColor.x, startColor.y, startColor.z, startColor.w };
	json["endColor"] = { endColor.x, endColor.y, endColor.z, endColor.w };
    return json;
}

void ParticleComponent::Deserialize(const nlohmann::json& json) {
	if (json.contains("modelName") && json["modelName"].is_string()) {
		modelName = json["modelName"].get<std::string>();
	}
	if (json.contains("maxParticleCount") && json["maxParticleCount"].is_number()) {
		const double value = json["maxParticleCount"].get<double>();
		maxParticleCount = static_cast<uint32_t>(std::clamp(value, 1.0, static_cast<double>(kMaxParticleCapacity)));
	}
	auto readFloat = [&json](const char* name, float& value) {
		if (json.contains(name) && json[name].is_number()) {
			value = json[name].get<float>();
		}
	};
	readFloat("minLifetime", minLifetime);
	readFloat("maxLifetime", maxLifetime);
	readFloat("minSpeed", minSpeed);
	readFloat("maxSpeed", maxSpeed);
	readFloat("startScale", startScale);
	readFloat("endScale", endScale);
	readFloat("scaleVariation", scaleVariation);
	readFloat("gravity", gravity);
	readFloat("directionSpread", directionSpread);
	if (json.contains("startColor")) {
		ReadColor(json["startColor"], startColor);
	}
	if (json.contains("endColor")) {
		ReadColor(json["endColor"], endColor);
	}
}

void ParticleComponent::InitializeResources(AssetManager& assetManager) {
	if (modelName.empty()) {
		modelName = "Box1x1.obj";
	}
	maxParticleCount = std::clamp(maxParticleCount, 1u, kMaxParticleCapacity);
	if (minLifetime > maxLifetime) {
		std::swap(minLifetime, maxLifetime);
	}
	if (minSpeed > maxSpeed) {
		std::swap(minSpeed, maxSpeed);
	}
	minLifetime = std::max(minLifetime, 0.01f);
	maxLifetime = std::max(maxLifetime, minLifetime);
	minSpeed = std::max(minSpeed, 0.0f);
	maxSpeed = std::max(maxSpeed, minSpeed);
	startScale = std::max(startScale, 0.0f);
	endScale = std::max(endScale, 0.0f);
	scaleVariation = std::clamp(scaleVariation, 0.0f, 1.0f);
	gravity = std::max(gravity, 0.0f);
	directionSpread = std::max(directionSpread, 0.0f);

	vartexBufferHandle = assetManager.LoadModelMesh(modelName);
	textureHandle = assetManager.LoadModelTexture(modelName);
	materialHandle = assetManager.GetGpuBufferPool()->AcquireConstantBuffer<Material>();
	particleGpuBufferHandle = assetManager.GetParticleGpuDataManager()->CreateParticleBuffer(maxParticleCount);
	nextParticleIndex = 0;
	particles.assign(maxParticleCount, ParticleRuntimeData{});
}

void ParticleComponent::EmitBurst(const Vector3& position, uint32_t count, const Vector3& direction) {
	if (maxParticleCount == 0 || particles.empty()) {
		return;
	}
	static thread_local std::mt19937 generator{ std::random_device{}() };
	auto randomFloat = [](float minValue, float maxValue) {
		return std::uniform_real_distribution<float>(minValue, maxValue)(generator);
	};

	const bool hasDirection = direction.LengthSq() > kMinDirectionLengthSquared;
	const Vector3 baseDirection = hasDirection ? direction.Normalize() : Vector3{ 0.0f, 0.0f, 0.0f };
	const float lifetimeMin = std::min(minLifetime, maxLifetime);
	const float lifetimeMax = std::max(minLifetime, maxLifetime);
	const float speedMin = std::min(minSpeed, maxSpeed);
	const float speedMax = std::max(minSpeed, maxSpeed);
	for (uint32_t emitted = 0; emitted < count; ++emitted) {
		ParticleRuntimeData& particle = particles[nextParticleIndex];
		nextParticleIndex = (nextParticleIndex + 1) % maxParticleCount;

		Vector3 velocityDirection;
		if (hasDirection) {
			const Vector3 spread{
				randomFloat(-directionSpread, directionSpread),
				randomFloat(-directionSpread, directionSpread),
				randomFloat(-directionSpread, directionSpread) };
			velocityDirection = (baseDirection + spread).Normalize();
			if (velocityDirection.LengthSq() <= kMinDirectionLengthSquared) {
				velocityDirection = baseDirection;
			}
		} else {
			Vector3 sample;
			float lengthSquared = 0.0f;
			do {
				sample = Vector3{
					randomFloat(-1.0f, 1.0f),
					randomFloat(-1.0f, 1.0f),
					randomFloat(-1.0f, 1.0f) };
				lengthSquared = sample.LengthSq();
			} while (lengthSquared <= kMinDirectionLengthSquared || lengthSquared > 1.0f);
			velocityDirection = sample.Normalize();
		}

		particle.position = position;
		particle.velocity = velocityDirection * randomFloat(speedMin, speedMax);
		particle.rotation = Vector3{
			randomFloat(-3.14159f, 3.14159f),
			randomFloat(-3.14159f, 3.14159f),
			randomFloat(-3.14159f, 3.14159f) };
		particle.angularVelocity = Vector3{
			randomFloat(-6.0f, 6.0f),
			randomFloat(-6.0f, 6.0f),
			randomFloat(-6.0f, 6.0f) };
		particle.age = 0.0f;
		particle.lifetime = randomFloat(lifetimeMin, lifetimeMax);
		particle.startScale = startScale * randomFloat(1.0f - scaleVariation, 1.0f + scaleVariation);
		particle.isActive = true;
	}
}

void ParticleComponent::Update(float deltaTime, ParticleForGPU* gpuData) {
	if (!gpuData) {
		return;
	}
	deltaTime = std::max(deltaTime, 0.0f);
	for (uint32_t i = 0; i < maxParticleCount; ++i) {
		ParticleRuntimeData& particle = particles[i];
		ParticleForGPU& renderData = gpuData[i];
		if (!particle.isActive) {
			renderData.color.w = 0.0f;
			continue;
		}

		particle.age += deltaTime;
		if (particle.age >= particle.lifetime) {
			particle.isActive = false;
			renderData.color.w = 0.0f;
			continue;
		}

		particle.velocity.y -= gravity * deltaTime;
		particle.position += particle.velocity * deltaTime;
		particle.rotation += particle.angularVelocity * deltaTime;
		const float t = std::clamp(particle.age / particle.lifetime, 0.0f, 1.0f);
		const float scale = particle.startScale + (endScale - particle.startScale) * t;
		Transform transform;
		transform.scale = Vector3{ scale, scale, scale };
		transform.rotate = particle.rotation;
		transform.translate = particle.position;
		renderData.World = Matrix4x4::MakeAffineMatrix(transform);
		renderData.color = Vector4{
			startColor.x + (endColor.x - startColor.x) * t,
			startColor.y + (endColor.y - startColor.y) * t,
			startColor.z + (endColor.z - startColor.z) * t,
			startColor.w + (endColor.w - startColor.w) * t };
	}
}
