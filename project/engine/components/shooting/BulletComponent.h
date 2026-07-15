#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <stdint.h>

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct BulletComponent {
		float speed;
		float lifeTime;
		float lifeTimeMax;
		QFE::MATH::Vector3 dir;
		bool isGravity = false;
		uint32_t damage = 1; // ダメージ量を追加

		uint32_t mask = 0xFFFFFFFF; // マスクの初期値を全ビット1に設定

		QFE_REFLECT_BEGIN(BulletComponent)
			QFE_REFLECT_MEMBER(speed)
			QFE_REFLECT_MEMBER(lifeTime)
			QFE_REFLECT_MEMBER(lifeTimeMax)
			QFE_REFLECT_MEMBER(dir)
			QFE_REFLECT_MEMBER(isGravity)
			QFE_REFLECT_MEMBER(mask)
			QFE_REFLECT_MEMBER(damage)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(BulletComponent)
}