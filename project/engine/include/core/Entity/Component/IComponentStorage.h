#pragma once
#include "engine/include/core/Entity/Component/ComponentData.h"

namespace QFE {

	class IComponentStorage {
	public:
		virtual ~IComponentStorage() = default;
		virtual void RemoveComponent(uint32_t id) = 0;
		virtual ComponentData* GetComponentDataPtr(uint32_t id) = 0;
	};

}
