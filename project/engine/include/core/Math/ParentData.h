#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"

namespace QFE {

	class ParentData final :public ComponentData {
	public:
		uint32_t parentId = 0;

		ParentData() = default;
		~ParentData() = default;
		nlohmann::json Serialize() const override {
			nlohmann::json json;
			json["parentId"] = parentId;
			return json;
		}
		void Deserialize(const nlohmann::json& json) override {
			if (json.contains("parentId")) parentId = json["parentId"].get<uint32_t>();
		}
		std::string GetTypeName() const override { return "ParentData"; }
	};

}
