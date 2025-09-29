#pragma once
#include <nlohmann/json.hpp>
#include "Vector/Vector3.h"
#include "Matrix/Matrix4x4.h"
#include "Core/Entity/Component/ComponentData.h"

class Transform final : public ComponentData {
public:
    Vector3 scale{ 1.0f, 1.0f, 1.0f };
    Vector3 rotate{ 0.0f, 0.0f, 0.0f };
    Vector3 translate{ 0.0f, 0.0f, 0.0f };

	void FromMatrix(const Matrix4x4& mat);
	nlohmann::json Serialize() const override;
    void Deserialize(const nlohmann::json& json) override;
    std::string GetTypeName() const override { return "Transform"; }
};