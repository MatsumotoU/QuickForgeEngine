#pragma once

#include <cstdint>
#include <vector>

#include "ComponentParameterSystem.h"

namespace QFE {
	class EntityManager;
}

namespace QFE::FRAMEWORK {
	void BeginCollisionTriggerFrame(EntityManager& entityManager);
	void NotifyCollisionTrigger(
		EntityManager& entityManager,
		uint32_t colliderEntityId,
		uint32_t otherEntityId,
		uint32_t otherCollisionMask,
		std::vector<ComponentParameterCommand>& commandBuffer);
	void EndCollisionTriggerFrame(
		EntityManager& entityManager,
		std::vector<ComponentParameterCommand>& commandBuffer);
}
