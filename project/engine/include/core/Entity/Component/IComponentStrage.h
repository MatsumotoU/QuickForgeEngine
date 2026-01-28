#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"

namespace QFE {

	class IComponentStrage {
	public:
		virtual ~IComponentStrage() = default;
		virtual void RemoveComponent(uint32_t id) = 0;
		virtual ComponentData* GetComponentDataPtr(uint32_t id) = 0;
	};

}
