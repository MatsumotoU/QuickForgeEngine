#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief TransformComponentは、オブジェクトの位置、回転、スケールを表すコンポーネントです。
	struct TransformComponent {
		MATH::Vector3 position; ///< オブジェクトの位置
		MATH::Vector3 rotation; ///< オブジェクトの回転
		MATH::Vector3 scale;    ///< オブジェクトのスケール

		QFE_REFLECT_BEGIN()
			QFE_REFLECT_MEMBER(position)
			QFE_REFLECT_MEMBER(rotation)
			QFE_REFLECT_MEMBER(scale)
		QFE_REFLECT_END()
	};
}