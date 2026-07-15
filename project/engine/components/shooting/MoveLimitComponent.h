#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <string>

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct MoveLimitComponent {
		QFE::MATH::Vector3 minLimit;
		QFE::MATH::Vector3 maxLimit;
		QFE::MATH::Vector3 center;
		QFE::MATH::Vector3 autoScrollSpeed;
		float autoScrollDistance = 0.0f;

		QFE_REFLECT_BEGIN(MoveLimitComponent)
			QFE_REFLECT_MEMBER(minLimit)
			QFE_REFLECT_MEMBER(maxLimit)
			QFE_REFLECT_MEMBER(center)
			QFE_REFLECT_MEMBER(autoScrollSpeed)
			QFE_REFLECT_MEMBER(autoScrollDistance)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(MoveLimitComponent)
}