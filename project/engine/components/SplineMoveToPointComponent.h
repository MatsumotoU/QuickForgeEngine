#pragma once

#include "EngineDefines.h"

#include <cstdint>

namespace QFE::SCENE {
	/// @brief Requests movement to a control point on the entity's spline path.
	struct SplineMoveToPointComponent {
		bool requestMoveToControlPoint = false;
		// Zero-based index in SplineMovementComponent::controlPoints.
		uint32_t targetControlPointIndex = 0;

		// Runtime state. These values are not serialized.
		bool movingToRequestedPoint = false;
		bool stoppedAtRequestedPoint = false;
		float requestedTravelTimeRemainingSeconds = 0.0f;
		float requestedTargetTimeSeconds = 0.0f;
		int32_t requestedTravelDirection = 1;

		QFE_REFLECT_BEGIN(SplineMoveToPointComponent)
			QFE_REFLECT_MEMBER(requestMoveToControlPoint)
			QFE_REFLECT_MEMBER(targetControlPointIndex)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SplineMoveToPointComponent)
}
