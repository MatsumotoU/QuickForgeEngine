#pragma once
#include "Core/Entity/Component/ComponentData.h"
#include "Core/Math/Vector/Vector2.h"
#include "Core/Math/Vector/Vector4.h"

class SpriteData final: public ComponentData {
public:
	SpriteData();
	~SpriteData() = default;
	
	nlohmann::json Serialize() const override;
	void Deserialize(const nlohmann::json& json) override;
	std::string GetTypeName() const override { return "SpriteData"; };

	std::string textureName;
	uint32_t textureHandle;
	uint32_t vertexBufferHandle;
	uint32_t wvpBufferHandle;
	uint32_t materialBufferHandle;
	uint32_t lightBufferHandle;
	uint32_t layer;
	float width;
	float height;
	Vector2 pivot;
	Vector4 color;

	bool isDraw;
};