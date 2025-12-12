#pragma once
#include "Engine/Core/Math/Shapes.h"
#include "Core/Entity/Component/ComponentData.h"
#include <nlohmann/json.hpp>

class SphereColliderData : public ComponentData {
public:
	Sphere sphere;
	bool isHit = false;
	bool isOldHit = false;

	bool isTrigger = false;
	bool isStatic = false; 
#ifdef _DEBUG
	bool isDraw = true;
#endif // _DEBUG
	uint8_t colliderLayer = 0xFF;
	uint8_t eventColliderLayer = 0xFF;

	SphereColliderData();
	~SphereColliderData() override = default;
	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	std::string GetTypeName() const override { return "SphereColliderData"; }
};