#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"
#include <string>

namespace QFE::SCENE {
	enum AnimationRequestFlags : uint32_t {
		AnimationRequestNone = 0,
		AnimationRequestPlay = 1u << 0,
		AnimationRequestStop = 1u << 1,
		AnimationRequestToggle = 1u << 2,
		AnimationRequestRestart = 1u << 3
	};

	/// @brief TransformComponentは、オブジェクトの位置、回転、スケールを表すコンポーネントです。
	struct AnimationComponent {
		std::string clipName;
		bool playOnStart = false;
		bool useClipLoop = true;
		bool loop = false;
		float playbackSpeed = 1.0f;
		uint32_t requestFlags = AnimationRequestNone;
		MATH::EulerTransform transform; ///< オブジェクトの位置、回転、スケールを表すEulerTransform構造体
		MATH::Vector4 color; ///< オブジェクトの色を表すVector4構造体

		// ランタイム状態（シーンには保存しない）。
		bool playing = false;
		bool initialized = false;
		float currentTime = 0.0f;

		QFE_REFLECT_BEGIN(AnimationComponent)
			QFE_REFLECT_MEMBER(clipName)
			QFE_REFLECT_MEMBER(playOnStart)
			QFE_REFLECT_MEMBER(useClipLoop)
			QFE_REFLECT_MEMBER(loop)
			QFE_REFLECT_MEMBER(playbackSpeed)
			QFE_REFLECT_MEMBER(requestFlags)
			QFE_REFLECT_MEMBER(transform)
			QFE_REFLECT_MEMBER(color)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(AnimationComponent)
}
