#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief TransformComponentは、オブジェクトの位置、回転、スケールを表すコンポーネントです。
	struct TransformComponent {
		MATH::Transform transform; ///< オブジェクトの位置、回転、スケールを表すTransform構造体

		QFE_REFLECT_BEGIN(TransformComponent)
			QFE_REFLECT_MEMBER(transform)
		QFE_REFLECT_END()
	};
	QFE_COMPONENT(TransformComponent)
}