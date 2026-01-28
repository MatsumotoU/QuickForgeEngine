#pragma once
#include <cstdint>
#include "engine/include/core/Math/Transform.h"

namespace QFE {

	enum class KeyFrameInterpolationType : uint8_t {
		LINEAR,
		BEZIER,
		STEP,
		EASE_IN,
		EASE_OUT
	};

	struct KeyFrame {
		float time;
		Transform transform;
		KeyFrameInterpolationType interpolationType;
	};

}
