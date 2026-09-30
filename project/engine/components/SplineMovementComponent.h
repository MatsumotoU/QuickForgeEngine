#pragma once

#include "EngineDefines.h"
#include "SplineControlPoint.h"

#include <cstdint>
#include <vector>

namespace QFE::SCENE {
	/// @brief 制御点のTransformに沿ってエンティティを移動させるスプライン設定。
	struct SplineMovementComponent {
		bool enabled = true;
		bool paused = false;
		bool autoPlay = true;
		bool loop = false;
		bool syncRotationToPath = true;
		bool drawPath = true;
		// Plays this entity's EventComponent after the spline ends and the delay expires.
		bool triggerEventAfterEnd = false;
		float eventDelayAfterEndSeconds = 0.0f;
		QFE::MATH::Vector4 pathColor{ 0.15f, 1.0f, 0.3f, 1.0f };
		std::vector<SplineControlPoint> controlPoints{ SplineControlPoint{} };

		// 実行時状態。シーンには保存しない。
		float elapsedTimeSeconds = 0.0f;
		float elapsedAfterEndSeconds = 0.0f;
		bool movementStarted = false;
		bool finished = false;
		bool endEventTriggered = false;

		QFE_REFLECT_BEGIN(SplineMovementComponent)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(paused)
			QFE_REFLECT_MEMBER(autoPlay)
			QFE_REFLECT_MEMBER(loop)
			QFE_REFLECT_MEMBER(syncRotationToPath)
			QFE_REFLECT_MEMBER(drawPath)
			QFE_REFLECT_MEMBER(triggerEventAfterEnd)
			QFE_REFLECT_MEMBER(eventDelayAfterEndSeconds)
			QFE_REFLECT_MEMBER(pathColor)
			QFE_REFLECT_MEMBER(controlPoints)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SplineMovementComponent)
}
