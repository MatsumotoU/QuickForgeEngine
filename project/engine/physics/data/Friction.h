#pragma once
namespace QFE::PHYSICS {
	/// @brief 物理エンジンにおける摩擦を表す構造体
	struct Friction {
		float staticFriction = 0.6f; ///< 静止摩擦係数
		float kineticFriction = 0.4f; ///< 動摩擦係数
	};
}
