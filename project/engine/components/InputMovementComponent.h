#pragma once

#include "EngineDefines.h"

#include <string>

namespace QFE::COMPONENTS {
	/// @brief PhysicsComponent があれば力を加え、なければ位置を直接動かす。
	struct InputMovementComponent {
		bool enabled = true;
		std::string rightActionName = "Right";
		std::string leftActionName = "Left";
		std::string forwardActionName = "Up";
		std::string backwardActionName = "Down";
		QFE::MATH::Vector3 horizontalAxis{ 1.0f, 0.0f, 0.0f };
		QFE::MATH::Vector3 verticalAxis{ 0.0f, 0.0f, 1.0f };
		float amount = 5.0f; ///< 物理なしでは速度、物理ありでは力の大きさ。
		bool normalizeInput = true;
		bool useGamePadLeftStick = true;

		QFE_REFLECT_BEGIN(InputMovementComponent)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(rightActionName)
			QFE_REFLECT_MEMBER(leftActionName)
			QFE_REFLECT_MEMBER(forwardActionName)
			QFE_REFLECT_MEMBER(backwardActionName)
			QFE_REFLECT_MEMBER(horizontalAxis)
			QFE_REFLECT_MEMBER(verticalAxis)
			QFE_REFLECT_MEMBER(amount)
			QFE_REFLECT_MEMBER(normalizeInput)
			QFE_REFLECT_MEMBER(useGamePadLeftStick)
		QFE_REFLECT_END()
	};

}
