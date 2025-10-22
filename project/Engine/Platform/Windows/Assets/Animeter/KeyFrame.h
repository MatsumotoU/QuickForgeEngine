#pragma once
#include <cstdint>
#include "Core/Math/Transform.h"

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