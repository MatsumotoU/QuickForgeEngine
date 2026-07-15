#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <stdint.h>

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct AutoScrollComponent {
		float speed;
		float distance;

		QFE_REFLECT_BEGIN(AutoScrollComponent)
			QFE_REFLECT_MEMBER(speed)
			QFE_REFLECT_MEMBER(distance)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(AutoScrollComponent)
}