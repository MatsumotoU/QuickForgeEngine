#pragma once
#include <cstdint>
#include <string>
#include "Core/Entity/Component/ComponentData.h"
#include "Core/Math/Vector/Vector4.h"

class ModelHandle final :public ComponentData {
public:
	uint32_t handle;
	std::string modelName;

	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;

	std::string GetTypeName() const override { return "ModelHandle"; }
};