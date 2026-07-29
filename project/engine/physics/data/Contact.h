#pragma once
#include "../PhysicsEngineTypes.h"
#include "math/MathInclude.h"

namespace QFE::PHYSICS {
	/// @brief 1組の球コライダーから生成された接触情報。
	struct Contact {
		ColliderHandle colliderA = ColliderHandle::Invalid;
		ColliderHandle colliderB = ColliderHandle::Invalid;
		RigidBodyHandle bodyA = RigidBodyHandle::Invalid;
		RigidBodyHandle bodyB = RigidBodyHandle::Invalid;
		QFE::MATH::Vector3 point{};
		/// bodyAからbodyBへ向かう単位法線。
		QFE::MATH::Vector3 normal{};
		float penetration = 0.0f;
		bool isTrigger = false;
	};
}
