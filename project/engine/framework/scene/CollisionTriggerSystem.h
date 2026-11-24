#pragma once

#include <cstdint>
#include <vector>
#include <functional>

#include "ComponentParameterSystem.h"

namespace QFE {
	class EntityManager;
}

namespace QFE::FRAMEWORK {
	struct CollisionTriggerCommand {
		uint32_t selfEntityId = UINT32_MAX;
		uint32_t otherEntityId = UINT32_MAX;
		nlohmann::json action;
	};
	void BeginCollisionTriggerFrame(EntityManager& entityManager);
	void NotifyCollisionTrigger(
		EntityManager& entityManager,
		uint32_t colliderEntityId,
		uint32_t otherEntityId,
		uint32_t otherCollisionMask,
		std::vector<CollisionTriggerCommand>& commandBuffer);
	void EndCollisionTriggerFrame(
		EntityManager& entityManager,
		std::vector<CollisionTriggerCommand>& commandBuffer);
	/// Spawn callback receives an assets-relative entity path and the selected target entity.
	void ApplyCollisionTriggerCommands(EntityManager& entityManager,
		const std::vector<CollisionTriggerCommand>& commands,
		const std::function<void(const std::string&, uint32_t)>& spawn);
}
