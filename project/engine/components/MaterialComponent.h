#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief オブジェクトの情報を表すコンポーネントです。
	struct MaterialComponent {
		MATH::Vector4 albedoColor; ///< アルベドカラー
		float metallic; ///< 金属度
		float smoothness; ///< 光沢度
		QFE::MATH::EulerTransform uvTransform; ///< UV

		uint32_t materialResourceHandle; ///< マテリアルリソースハンドル

		QFE_REFLECT_BEGIN(MaterialComponent)
			QFE_REFLECT_MEMBER(albedoColor)
			QFE_REFLECT_MEMBER(metallic)
			QFE_REFLECT_MEMBER(smoothness)
			QFE_REFLECT_MEMBER(uvTransform)
			QFE_REFLECT_END()
	};

	QFE_COMPONENT(MaterialComponent)
}

