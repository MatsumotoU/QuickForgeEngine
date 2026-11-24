#pragma once
#include <cstdint>
#include "math/vector/Vector3.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	// Half sizes in world units; Transform rotation is applied but scale is not.
	struct OBBColliderComponent {
		MATH::Vector3 halfExtents{ 0.5f, 0.5f, 0.5f };
		uint32_t mask = 0;
		QFE_REFLECT_BEGIN(OBBColliderComponent)
			QFE_REFLECT_MEMBER(halfExtents)
			QFE_REFLECT_MEMBER(mask)
		QFE_REFLECT_END()
	};
}
