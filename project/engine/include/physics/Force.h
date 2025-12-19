#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"
#include "engine/include/core/Math/Vector/Vector3.h"

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

	void AddForce(const Vector3& force);

	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	std::string GetTypeName() const override;
};
