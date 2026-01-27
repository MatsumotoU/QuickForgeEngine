#pragma once
#include <cstdint>
#include <string>
#include "engine/include/core/Entity/Component/ComponentData.h"
#include "engine/include/core/Math/Vector/Vector4.h"

namespace QFE {

	class ModelHandle final :public ComponentData {
	public:
		uint32_t handle;
		std::string modelName;

		nlohmann::json Serialize() const override;
		void Deserialize(const nlohmann::json& json) override;

		std::string GetTypeName() const override { return "ModelHandle"; }
	};

}
