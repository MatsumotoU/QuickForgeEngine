#pragma once
#include <stdint.h>

namespace QFE {
	class EntityManager;
}

namespace QFE::Render::Sprite {
	void DrawSprites(EntityManager* entityManager, const uint32_t& spriteHandle);
}
