#pragma once
#include <cstdint>
#include "math/vector/Vector3.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	// Half sizes in world units; Transform scale and rotation are not applied.
	struct AABBColliderComponent {
		MATH::Vector3 halfExtents{ 0.5f, 0.5f, 0.5f };
		uint32_t mask = 0;
		QFE_REFLECT_BEGIN(AABBColliderComponent)
			QFE_REFLECT_MEMBER(halfExtents)
			QFE_REFLECT_MEMBER(mask)
		QFE_REFLECT_END()
	};
}
