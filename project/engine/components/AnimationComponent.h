#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief TransformComponentは、オブジェクトの位置、回転、スケールを表すコンポーネントです。
	struct AnimationComponent {
		MATH::EulerTransform transform; ///< オブジェクトの位置、回転、スケールを表すEulerTransform構造体
		MATH::Vector4 color; ///< オブジェクトの色を表すVector4構造体

		QFE_REFLECT_BEGIN(AnimationComponent)
			QFE_REFLECT_MEMBER(transform)
			QFE_REFLECT_MEMBER(color)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(AnimationComponent)
}