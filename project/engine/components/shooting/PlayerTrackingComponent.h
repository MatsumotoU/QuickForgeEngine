#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <string>

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct PlayerTrackingComponent {
		QFE::MATH::Vector3 trackingOffsetPos;
		bool isTrackingX = true;
		bool isTrackingY = true;
		bool isTrackingZ = true;
		bool isTrackingRotation = false;
		QFE::MATH::Vector3 trackingRotationOffset;
		float trackingRotationTranspose = 1.0f;

		QFE_REFLECT_BEGIN(PlayerTrackingComponent)
			QFE_REFLECT_MEMBER(trackingOffsetPos)
			QFE_REFLECT_MEMBER(isTrackingX)
			QFE_REFLECT_MEMBER(isTrackingY)
			QFE_REFLECT_MEMBER(isTrackingZ)
			QFE_REFLECT_MEMBER(isTrackingRotation)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(PlayerTrackingComponent)
}