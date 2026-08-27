#pragma once

#include "EngineDefines.h"

namespace QFE::STG {
	/// @brief 一定間隔で同じEntityのエミッターを起動する。
	struct PeriodicBulletEmitterTriggerComponent {
		bool enabled = true;
		bool emitOnStart = true;
		float interval = 1.0f;

		// ランタイム状態（シーンには保存しない）。
		bool initialized = false;
		float remainingTime = 0.0f;

		QFE_REFLECT_BEGIN(PeriodicBulletEmitterTriggerComponent)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(emitOnStart)
			QFE_REFLECT_MEMBER(interval)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(PeriodicBulletEmitterTriggerComponent)
}
