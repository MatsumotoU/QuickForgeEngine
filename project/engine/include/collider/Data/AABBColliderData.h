#pragma once
#include "engine/include/core/Math/Shapes.h"
#include "engine/include/core/Entity/Component/ComponentData.h"
#include <nlohmann/json.hpp>

namespace QFE {

	class AABBColliderData final: public ComponentData {
	public:
		AABB aabb;
		bool isHit = false;
		bool isOldHit = false;
		bool isTrigger = false;
		bool isStatic = false;
		uint8_t colliderLayer = 0xFF;
		uint8_t eventColliderLayer = 0xFF;
	#ifdef QFE_OPTIMIZE_OFF
		bool isDraw = true;
	#endif // QFE_OPTIMIZE_OFF
		AABBColliderData();
		~AABBColliderData() override = default;
		nlohmann::json Serialize() const override;
		void Deserialize(const nlohmann::json& json) override;
		std::string GetTypeName() const override { return "AABBColliderData"; }
	};

}
