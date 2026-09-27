#pragma once

#include "math/MathInclude.h"

namespace QFE::SCENE {
	/// @brief Spline control point and duration to the next point.
	struct SplineControlPoint {
		// rotate is an Euler rotation offset from the spline's facing direction.
		QFE::MATH::EulerTransform transform{};
		float secondsToNextPoint = 1.0f;
	};
}
