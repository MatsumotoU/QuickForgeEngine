#pragma once
#include "math/MathInclude.h"

namespace QFE::PHYSICS {
	/// @brief 物理エンジンにおける力を表す構造体
	struct Force {
		QFE::MATH::Vector3 velocity{}; ///< 力の方向と大きさを表すベクトル
		QFE::MATH::Vector3 acceleration{}; ///< 力の加速度を表すベクトル
	};
}
