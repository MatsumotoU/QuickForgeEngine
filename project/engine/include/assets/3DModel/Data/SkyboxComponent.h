#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"

namespace QFE {
	class SkyboxComponent final :public ComponentData {
	public:
		uint32_t textureHandle = UINT32_MAX;
		uint32_t vertexBufferHandle = UINT32_MAX;
		uint32_t wvpBufferHandle = UINT32_MAX;
		uint32_t materialBufferHandle = UINT32_MAX;

		std::string textureName;

		nlohmann::json Serialize() const override;
		void Deserialize(const nlohmann::json& json) override;
		std::string GetTypeName() const override { return "SkyboxComponent"; }
	};
}
