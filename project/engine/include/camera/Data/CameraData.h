#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"

namespace QFE {
	class CameraData : public ComponentData {
	public:
		uint32_t handle_;
		float fovY_;      // 垂直視野角
		float nearZ_;     // 前方クリップ距離
		float farZ_;      // 後方クリップ距離

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
}
