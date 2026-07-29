#pragma once
#include "Friction.h"

namespace QFE::PHYSICS {
	/// @brief 物理エンジンにおける材質を表す構造体
	struct PhysicsMaterial {
		Friction friction{}; ///< 摩擦係数
		float restitution = 0.2f; ///< 反発係数

		bool isGravityEnabled = true; ///< 重力の影響を受けるかどうか
	};
}
