#pragma once
#include "Engine/Core/Math/Shapes.h"
#include "Core/Entity/Component/ComponentData.h"
#include <nlohmann/json.hpp>

class AABBColliderData final: public ComponentData {
public:
	AABB aabb;
	bool isHit = false;
	bool isOldHit = false;
	bool isTrigger = false;
	bool isStatic = false;
	uint8_t colliderLayer = 0xFF;
	uint8_t eventColliderLayer = 0xFF;
#ifdef _DEBUG
	bool isDraw = true;
#endif // _DEBUG
	AABBColliderData();
	~AABBColliderData() override = default;
	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	std::string GetTypeName() const override { return "AABBColliderData"; }
};