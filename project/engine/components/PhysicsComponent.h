#pragma once
#include "EngineDefines.h"
#include "design-patterns/component/EntityReference.h"

namespace QFE::COMPONENTS {
	/// @brief 物理演算用のコンポーネント
	struct PhysicsComponent {
		QFE::MATH::Vector3 velocity;
		QFE::MATH::Vector3 acceleration;
		float mass;
		float friction;

		QFE_REFLECT_BEGIN(PhysicsComponent)
			QFE_REFLECT_MEMBER(velocity)
			QFE_REFLECT_MEMBER(acceleration)
			QFE_REFLECT_MEMBER(mass)
			QFE_REFLECT_MEMBER(friction)
		QFE_REFLECT_END()
	};

	QFE_COMPONENT(PhysicsComponent)
}