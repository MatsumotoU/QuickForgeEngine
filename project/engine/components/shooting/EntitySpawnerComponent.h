#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <string>

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct EntitySpawnerComponent {
		float spawnTriggerDistance = 10.0f; // スポーントリガー距離
		std::string entityPrefabName; // スポーンするエンティティのプレハブ名
		QFE::MATH::Bit32 spawnMask; // スポーンマスク。どのスポーントリガーに反応するかを示すビットフラグ。

		QFE_REFLECT_BEGIN(EntitySpawnerComponent)
			QFE_REFLECT_MEMBER(spawnTriggerDistance)
			QFE_REFLECT_MEMBER(entityPrefabName)
			QFE_REFLECT_MEMBER(spawnMask)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(EntitySpawnerComponent)
}