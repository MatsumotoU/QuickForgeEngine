#pragma once

#include <cstdint>

namespace QFE {
	class EntityManager;
}

namespace QFE::FRAMEWORK {
	bool PlayAnimation(EntityManager& entityManager, uint32_t entityId, bool restart = true);
	bool StopAnimation(EntityManager& entityManager, uint32_t entityId, bool resetTime = false);
	void UpdateAnimationComponents(EntityManager& entityManager, float deltaTime);
}
