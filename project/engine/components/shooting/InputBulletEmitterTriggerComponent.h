#pragma once

#include "EngineDefines.h"

#include <cstdint>
#include <string>

namespace QFE::STG {
	enum BulletEmitterInputTriggerMode : uint32_t {
		BulletEmitterInputPress = 0,
		BulletEmitterInputTrigger = 1,
		BulletEmitterInputRelease = 2
	};

	/// @brief キーボード、マウス、ゲームパッド入力で同じEntityのエミッターを起動する。
	struct InputBulletEmitterTriggerComponent {
		bool enabled = true;
		std::string inputActionName = "Shot"; ///< 空文字ならキーボード入力を使用しない
		int32_t mouseButton = -1; ///< 0以上なら使用する
		uint32_t gamePadButton = 0; ///< XInputボタンマスク。0なら使用しない
		uint32_t triggerMode = BulletEmitterInputPress;
		float repeatInterval = 0.2f; ///< Press時の連続起動間隔。0以下なら毎フレーム

		// ランタイム状態（シーンには保存しない）。
		float repeatTimer = 0.0f;

		QFE_REFLECT_BEGIN(InputBulletEmitterTriggerComponent)
			QFE_REFLECT_MEMBER(enabled)
			QFE_REFLECT_MEMBER(inputActionName)
			QFE_REFLECT_MEMBER(mouseButton)
			QFE_REFLECT_MEMBER(gamePadButton)
			QFE_REFLECT_MEMBER(triggerMode)
			QFE_REFLECT_MEMBER(repeatInterval)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(InputBulletEmitterTriggerComponent)
}
