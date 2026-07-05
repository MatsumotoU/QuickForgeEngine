#pragma once
#include "ComponentData.h"
#include <functional>

namespace QFE {

	class IComponentStorage {
	public:
		virtual ~IComponentStorage() = default;
		virtual void RemoveComponent(uint32_t id) = 0;
		virtual ComponentData* GetComponentDataPtr(uint32_t id) = 0;
	};

}
