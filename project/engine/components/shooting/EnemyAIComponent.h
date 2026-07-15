#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <stdint.h>
#include <string>

namespace QFE::STG {
	/// @brief シューティングゲームの敵AIコンポーネントの情報を保持する構造体
	struct EnemyAIComponent {
		std::string bulletName;
		float shotInterval;
		float shotTimer;

		QFE_REFLECT_BEGIN(EnemyAIComponent)
			QFE_REFLECT_MEMBER(bulletName)
			QFE_REFLECT_MEMBER(shotInterval)
			QFE_REFLECT_MEMBER(shotTimer)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(EnemyAIComponent)
}