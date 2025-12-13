#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"

class CameraData : public ComponentData {
public:
	uint32_t handle_;
	float fovY_;      // 蝙ら峩隕夜㍽隗・
	float nearZ_;     // 蜑肴婿繧ｯ繝ｪ繝・・霍晞屬
	float farZ_;      // 蠕梧婿繧ｯ繝ｪ繝・・霍晞屬

	CameraData() = default;
	virtual ~CameraData() = default;

	nlohmann::json Serialize() const override {
		nlohmann::json json;
		json["fovY"] = fovY_;
		json["nearZ"] = nearZ_;
		json["farZ"] = farZ_;
		return json;
	}
	void Deserialize(const nlohmann::json& json) override {
		if (json.contains("fovY")) {
			fovY_ = json["fovY"].get<float>();
		}
		if (json.contains("nearZ")) {
			nearZ_ = json["nearZ"].get<float>();
		}
		if (json.contains("farZ")) {
			farZ_ = json["farZ"].get<float>();
		}
	}
	std::string GetTypeName() const override { return "CameraData"; }
};
