#include "engine/include/assets/3DModel/Data/SkyboxComponent.h"
#include "engine/include/assets/AssetManager.h"

nlohmann::json QFE::SkyboxComponent::Serialize() const
{
    nlohmann::json json;
	json["textureName"] = textureName;
	return json;
}

void QFE::SkyboxComponent::Deserialize(const nlohmann::json& json)
{
	if (json.contains("textureName") && json["textureName"].is_string()) {
		textureName = json["textureName"].get<std::string>();
	}
}
