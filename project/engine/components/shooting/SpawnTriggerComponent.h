#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <string>

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct SpawnTriggerComponent {
		QFE::MATH::Bit32 spawnMask; ///< スポーンマスク。どのスポーントリガーに反応するかを示すビットフラグ。

		QFE_REFLECT_BEGIN(SpawnTriggerComponent)
			QFE_REFLECT_MEMBER(spawnMask)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(SpawnTriggerComponent)
}