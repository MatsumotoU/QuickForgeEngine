#pragma once
#include <nlohmann/json.hpp>
#include "Vector/Vector3.h"
#include "Core/Entity/Component/ComponentData.h"

class Transform final : public ComponentData {
public:
    Vector3 scale{ 1.0f, 1.0f, 1.0f };
    Vector3 rotate{ 0.0f, 0.0f, 0.0f };
    Vector3 translate{ 0.0f, 0.0f, 0.0f };

    nlohmann::json Serialize() const override {
        nlohmann::json json;
        json["scale"] = { scale.x, scale.y, scale.z };
        json["rotate"] = { rotate.x, rotate.y, rotate.z };
        json["translate"] = { translate.x, translate.y, translate.z };
        return json;
    }
	void Deserialize(const nlohmann::json& json) override {
		if (json.contains("scale") && json["scale"].is_array() && json["scale"].size() == 3) {
			scale.x = json["scale"][0].get<float>();
			scale.y = json["scale"][1].get<float>();
			scale.z = json["scale"][2].get<float>();
		}
		if (json.contains("rotate") && json["rotate"].is_array() && json["rotate"].size() == 3) {
			rotate.x = json["rotate"][0].get<float>();
			rotate.y = json["rotate"][1].get<float>();
			rotate.z = json["rotate"][2].get<float>();
		}
		if (json.contains("translate") && json["translate"].is_array() && json["translate"].size() == 3) {
			translate.x = json["translate"][0].get<float>();
			translate.y = json["translate"][1].get<float>();
			translate.z = json["translate"][2].get<float>();
		}
	}
    std::string GetTypeName() const override { return "Transform"; }
};