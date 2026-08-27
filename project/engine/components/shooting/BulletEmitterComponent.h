#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <cstdint>
#include <string>

namespace QFE::STG {
	/// @brief 球面上に弾丸を配置して発射するエミッター。
	struct BulletEmitterComponent {
		std::string emitBulletName = "Bullet.json"; ///< 発射する弾丸Prefab
		QFE::MATH::Vector3 emitPos = { 0.0f, 0.0f, 0.0f }; ///< エミッターから見た球の中心位置
		QFE::MATH::Vector3 emitDir = { 0.0f, 0.0f, 1.0f }; ///< 0番目の弾丸が向く基準方向
		float emitRadius = 0.0f; ///< 球の中心から弾丸を配置する距離

		uint32_t emitCount = 1; ///< 1回に発射する弾丸数
		float bulletAngleX = 0.0f; ///< 弾丸ごとに加算する極角theta（ラジアン）
		float bulletAngleY = 0.0f; ///< 弾丸ごとに加算する方位角phi（ラジアン）

		bool emitRequest = false; ///< 発射要求フラグ

		QFE_REFLECT_BEGIN(BulletEmitterComponent)
			QFE_REFLECT_MEMBER(emitBulletName)
			QFE_REFLECT_MEMBER(emitPos)
			QFE_REFLECT_MEMBER(emitDir)
			QFE_REFLECT_MEMBER(emitRadius)
			QFE_REFLECT_MEMBER(emitCount)
			QFE_REFLECT_MEMBER(bulletAngleX)
			QFE_REFLECT_MEMBER(bulletAngleY)
			QFE_REFLECT_MEMBER(emitRequest)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(BulletEmitterComponent)
}
