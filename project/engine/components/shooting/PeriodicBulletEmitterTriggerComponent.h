#pragma once

#include "EngineDefines.h"

#include <cstdint>
#include <vector>

namespace QFE::STG {
	/// @brief 一定間隔で起動する弾幕トリガー1件分の設定。
	struct PeriodicBulletEmitterTriggerSetting {
		bool enabled = true;
		bool emitOnStart = true;
		float interval = 1.0f;
		uint32_t patternIndex = 0; ///< 起動するBulletEmitterComponentのパターン番号

		// ランタイム状態（シーンには保存しない）。
		bool initialized = false;
		float remainingTime = 0.0f;

		QFE_REFLECT_BEGIN(PeriodicBulletEmitterTriggerSetting)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(emitOnStart)
			QFE_REFLECT_MEMBER(interval)
			QFE_REFLECT_MEMBER(patternIndex)
		QFE_REFLECT_END()
	};

	/// @brief 複数の周期設定から、選択した弾幕パターンを起動する。
	struct PeriodicBulletEmitterTriggerComponent {
		std::vector<PeriodicBulletEmitterTriggerSetting> triggers{ PeriodicBulletEmitterTriggerSetting{} };

		QFE_REFLECT_BEGIN(PeriodicBulletEmitterTriggerComponent)
			QFE_REFLECT_MEMBER(triggers)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(PeriodicBulletEmitterTriggerComponent)
}
