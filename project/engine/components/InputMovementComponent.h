#pragma once

#include "EngineDefines.h"

#include <cstdint>
#include <string>

namespace QFE::COMPONENTS {
	enum InputMovementApplyMode : uint32_t {
		InputMovementPosition = 0,
		InputMovementForce = 1
	};

	/// @brief キーボードまたはゲームパッド入力を位置移動か力へ変換するコンポーネント。
	struct InputMovementComponent {
		bool enabled = true;
		std::string rightActionName = "Right";
		std::string leftActionName = "Left";
		std::string forwardActionName = "Up";
		std::string backwardActionName = "Down";
		QFE::MATH::Vector3 horizontalAxis{ 1.0f, 0.0f, 0.0f };
		QFE::MATH::Vector3 verticalAxis{ 0.0f, 0.0f, 1.0f };
		float amount = 5.0f; ///< Position時は速度、Force時は力の大きさ。
		uint32_t applyMode = InputMovementPosition;
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
			QFE_REFLECT_MEMBER(applyMode)
			QFE_REFLECT_MEMBER(normalizeInput)
			QFE_REFLECT_MEMBER(useGamePadLeftStick)
		QFE_REFLECT_END()
	};

	QFE_COMPONENT(InputMovementComponent)
}
