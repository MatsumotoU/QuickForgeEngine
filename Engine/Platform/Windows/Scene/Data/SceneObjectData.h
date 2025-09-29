#pragma once
#include <string>
#include "Core/Entity/Component/ComponentData.h"

class SceneObjectData final :public ComponentData {
public:
	std::string name;
	std::string tag;

	nlohmann::json Serialize() const override {
		nlohmann::json json;
		json["name"] = name;
		json["tag"] = tag;
		return json;
	}
	void Deserialize(const nlohmann::json& json) override {
		if (json.contains("name") && json["name"].is_string()) {
			name = json["name"].get<std::string>();
		}
		if (json.contains("tag") && json["tag"].is_string()) {
			tag = json["tag"].get<std::string>();
		}
	}
	std::string GetTypeName() const override { return "SceneObjectData"; }
};