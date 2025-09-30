#pragma once
#include "Core/Entity/Component/ComponentData.h"
#include "Core/Math/Vector/Vector3.h"

class Force final : public ComponentData {
public:
	Vector3 velocity;
	Vector3 acceleration;
	float mass;
	float friction;
	float gravityStrength;
	bool isGravity;

	Force();
	~Force() override = default;

	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	std::string GetTypeName() const override;
};