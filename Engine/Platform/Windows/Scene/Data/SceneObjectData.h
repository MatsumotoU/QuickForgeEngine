#pragma once
#include <string>
#include "Core/Entity/Component/ComponentData.h"

class SceneObjectData final :public ComponentData {
public:
	std::string name;
	std::string tag;
	uint32_t uniqueId;

	nlohmann::json Serialize() const override {
		nlohmann::json json;
		json["name"] = name;
		json["tag"] = tag;
		json["uniqueId"] = uniqueId;
		return json;
	}
	void Deserialize(const nlohmann::json& json) override {
		if (json.contains("name") && json["name"].is_string()) {
			name = json["name"].get<std::string>();
		}
		if (json.contains("tag") && json["tag"].is_string()) {
			tag = json["tag"].get<std::string>();
		}
		if (json.contains("uniqueId") && json["uniqueId"].is_number_unsigned()) {
			uniqueId = json["uniqueId"].get<uint32_t>();
		}
	}
	std::string GetTypeName() const override { return "SceneObjectData"; }
};