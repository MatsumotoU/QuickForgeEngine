#pragma once

#include "EngineDefines.h"

#include <vector>

namespace QFE::SCENE {
	/// @brief 制御点のTransformに沿ってエンティティを移動させるスプライン設定。
	struct SplineMovementComponent {
		bool enabled = true;
		bool drawPath = true;
		float speed = 2.0f; ///< ワールド空間での移動速度（単位/秒）。
		QFE::MATH::Vector4 pathColor{ 0.15f, 1.0f, 0.3f, 1.0f };
		std::vector<QFE::MATH::EulerTransform> controlPoints{ QFE::MATH::EulerTransform{} };

		// 実行時状態。シーンには保存しない。
		float distanceAlongPath = 0.0f;
		bool movementStarted = false;
		bool finished = false;

		QFE_REFLECT_BEGIN(SplineMovementComponent)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(drawPath)
			QFE_REFLECT_MEMBER(speed)
			QFE_REFLECT_MEMBER(pathColor)
			QFE_REFLECT_MEMBER(controlPoints)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(SplineMovementComponent)
}
