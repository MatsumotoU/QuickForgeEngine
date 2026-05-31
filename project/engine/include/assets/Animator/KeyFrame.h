#pragma once
#include <cstdint>
#include "engine/include/core/Math/Transform.h"

namespace QFE {
	struct KeyFrame {
		float time;
		EulerTransform transform;
	};
}
