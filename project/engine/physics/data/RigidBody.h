#pragma once
#include "math/MathInclude.h"

namespace QFE::PHYSICS {
	/// @brief 剛体を生成するときに使用する設定。
	struct RigidBodyDesc {
		QFE::MATH::Vector3 position{};
		QFE::MATH::Vector3 velocity{};
		float mass = 1.0f;
		/// 1秒間に残る速度の割合。1.0なら減衰しない。
		float linearDamping = 0.98f;
		float gravityScale = 1.0f;
		bool useGravity = true;
		bool isStatic = false;
	};

	/// @brief 並進運動だけを扱う最小構成の剛体。
	struct RigidBody {
		QFE::MATH::Vector3 position{};
		QFE::MATH::Vector3 velocity{};
		QFE::MATH::Vector3 accumulatedForce{};
		float inverseMass = 1.0f;
		float linearDamping = 0.98f;
		float gravityScale = 1.0f;
		bool useGravity = true;

		[[nodiscard]] bool IsStatic() const {
			return inverseMass <= 0.0f;
		}

		[[nodiscard]] float GetMass() const {
			return IsStatic() ? 0.0f : 1.0f / inverseMass;
		}
	};
}
