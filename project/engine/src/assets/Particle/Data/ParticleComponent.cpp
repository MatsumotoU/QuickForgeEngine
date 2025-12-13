#include "engine/include/assets/Particle/Data/ParticleComponent.h"

nlohmann::json ParticleComponent::Serialize() const {
	nlohmann::json json;
	json["modelName"] = modelName;
	json["maxParticleCount"] = maxParticleCount;
    return json;
}

void ParticleComponent::Deserialize(const nlohmann::json& json) {
	if (json.contains("modelName") && json["modelName"].is_string()) {
		modelName = json["modelName"].get<std::string>();
	}
	if (json.contains("maxParticleCount") && json["maxParticleCount"].is_number_unsigned()) {
		maxParticleCount = json["maxParticleCount"].get<uint32_t>();
	}
}
