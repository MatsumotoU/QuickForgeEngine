#pragma once

#include "EngineDefines.h"

#include <cstdint>

namespace QFE::STG {
	/// @brief プレイヤーと敵の両方で使う現在の HP と攻撃力。
	struct StatusComponent {
		uint32_t health = 3;
		uint32_t attackPower = 1;

		// シーン保存とエディターでの設定に使う。
		QFE_REFLECT_BEGIN(StatusComponent)
			QFE_REFLECT_MEMBER(health)
			QFE_REFLECT_MEMBER(attackPower)
		QFE_REFLECT_END()
	};
}
