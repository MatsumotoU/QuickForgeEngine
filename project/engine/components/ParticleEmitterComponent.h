#pragma once

#include "EngineDefines.h"

#include <cstdint>
#include <string>

namespace QFE::SCENE {
	/// @brief GPU上で更新・生成されるパーティクルのエミッタ設定。
	struct ParticleEmitterComponent {
		std::string textureName; ///< 空の場合はWhite1x1を使用する。
		bool emitting = true; ///< 新しいパーティクルを生成するか。
		bool oneShot = false; ///< trueになったフレームだけmaxParticles個を生成し、自動的にfalseへ戻る。
		bool visible = true; ///< パーティクルを描画するか。
		uint32_t maxParticles = 1024;
		float emissionRate = 64.0f; ///< 1秒あたりの生成数。
		float emitterRadius = 0.25f;
		MATH::Vector3 velocityMin{ -1.0f, 1.0f, -1.0f };
		MATH::Vector3 velocityMax{ 1.0f, 3.0f, 1.0f };
		MATH::Vector3 acceleration{ 0.0f, -1.0f, 0.0f };
		float lifetimeMin = 1.0f;
		float lifetimeMax = 2.0f;
		float startSize = 0.2f;
		float endSize = 0.0f;
		float angularVelocityMin = -1.0f;
		float angularVelocityMax = 1.0f;
		MATH::Vector4 startColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		MATH::Vector4 endColor{ 1.0f, 1.0f, 1.0f, 0.0f };
		uint32_t randomSeed = 1;

		bool canRender = false;
		uint32_t textureResourceHandle = UINT32_MAX;
		std::string renderErrorMessage;

		QFE_REFLECT_BEGIN(ParticleEmitterComponent)
			QFE_REFLECT_MEMBER(textureName)
			QFE_REFLECT_MEMBER(emitting)
			QFE_REFLECT_MEMBER(oneShot)
			QFE_REFLECT_MEMBER(visible)
			QFE_REFLECT_MEMBER(maxParticles)
			QFE_REFLECT_MEMBER(emissionRate)
			QFE_REFLECT_MEMBER(emitterRadius)
			QFE_REFLECT_MEMBER(velocityMin)
			QFE_REFLECT_MEMBER(velocityMax)
			QFE_REFLECT_MEMBER(acceleration)
			QFE_REFLECT_MEMBER(lifetimeMin)
			QFE_REFLECT_MEMBER(lifetimeMax)
			QFE_REFLECT_MEMBER(startSize)
			QFE_REFLECT_MEMBER(endSize)
			QFE_REFLECT_MEMBER(angularVelocityMin)
			QFE_REFLECT_MEMBER(angularVelocityMax)
			QFE_REFLECT_MEMBER(startColor)
			QFE_REFLECT_MEMBER(endColor)
			QFE_REFLECT_MEMBER(randomSeed)
		QFE_REFLECT_END()
	};
}
