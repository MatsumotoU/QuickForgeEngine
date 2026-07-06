#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief オブジェクトの情報を表すコンポーネントです。
	struct MaterialComponent {
		MATH::Vector4 albedoColor; ///< アルベドカラー

		uint32_t materialResourceHandle; ///< マテリアルリソースハンドル

		QFE_REFLECT_BEGIN(MaterialComponent)
			QFE_REFLECT_MEMBER(albedoColor)
			QFE_REFLECT_END()
	};
}