#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::STG {
	/// @brief シューティングゲームのプレイヤーコンポーネントの情報を保持する構造体
	struct ShootingPlayerComponent {
		float speed;
		float shootInterval;
		float shootTimer;
		float bombInterval;
		float bombTimer;
		QFE::MATH::Vector3 bulletSpawnOffset;
		QFE::MATH::Vector3 bombSpawnOffset;
		QFE::MATH::Vector3 velocity;
		// 止まりやすさ
		float damping = 0.85f;

		QFE_REFLECT_BEGIN(ShootingPlayerComponent)
			QFE_REFLECT_MEMBER(speed)
			QFE_REFLECT_MEMBER(shootInterval)
			QFE_REFLECT_MEMBER(bombInterval)
			QFE_REFLECT_MEMBER(bulletSpawnOffset)
			QFE_REFLECT_MEMBER(bombSpawnOffset)
			QFE_REFLECT_MEMBER(damping)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(ShootingPlayerComponent)
}