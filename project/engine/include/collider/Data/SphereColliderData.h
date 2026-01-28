#pragma once
#include "engine/include/core/Math/Shapes.h"
#include "engine/include/core/Entity/Component/ComponentData.h"
#include <nlohmann/json.hpp>

namespace QFE {

	class SphereColliderData : public ComponentData {
	public:
		Sphere sphere;
		bool isHit = false;
		bool isOldHit = false;

		bool isTrigger = false;
		bool isStatic = false; 
	#ifdef QFE_OPTIMIZE_OFF
		bool isDraw = true;
	#endif // QFE_OPTIMIZE_OFF
		uint8_t colliderLayer = 0xFF;
		uint8_t eventColliderLayer = 0xFF;

		SphereColliderData();
		~SphereColliderData() override = default;
		nlohmann::json Serialize() const override;
		void Deserialize(const nlohmann::json& json) override;
		std::string GetTypeName() const override { return "SphereColliderData"; }
	};

}
