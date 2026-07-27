#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <string>

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct HealthComponent {
		uint32_t health = 3;
		uint32_t maxHealth = 3;

		QFE_REFLECT_BEGIN(HealthComponent)
			QFE_REFLECT_MEMBER(health)
			QFE_REFLECT_MEMBER(maxHealth)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(HealthComponent)
}