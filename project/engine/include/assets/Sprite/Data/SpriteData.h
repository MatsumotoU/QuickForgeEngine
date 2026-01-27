#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"
#include "engine/include/core/Math/Vector/Vector2.h"

namespace QFE {
	class SpriteData final : public ComponentData {
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

		bool isDraw;
	};
}