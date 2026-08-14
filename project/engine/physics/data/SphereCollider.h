#pragma once
#include "../PhysicsEngineTypes.h"
#include "PhysicsMaterial.h"
#include "math/MathInclude.h"

namespace QFE::PHYSICS {
	/// @brief 球コライダーを生成するときに使用する設定。
	struct SphereColliderDesc {
		QFE::MATH::Vector3 localCenter{};
		float radius = 0.5f;
		PhysicsMaterial material{};
		bool isTrigger = false;
	};

	/// @brief 剛体に取り付けられる球コライダー。
	struct SphereCollider {
		RigidBodyHandle body = RigidBodyHandle::Invalid;
		QFE::MATH::Vector3 localCenter{};
		float radius = 0.5f;
		PhysicsMaterial material{};
		bool isTrigger = false;
	};
}
