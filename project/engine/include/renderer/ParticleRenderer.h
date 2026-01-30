#pragma once
#include <stdint.h>

namespace QFE {
	class EntityManager;
}

namespace QFE::Render::Particle {
	void DrawParticles(EntityManager* entityManager, const uint32_t& particleHandle);
}
