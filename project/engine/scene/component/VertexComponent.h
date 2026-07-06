#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief 頂点データを表すコンポーネントです。
	struct VertexComponent {
		std::string vertexName; ///< 頂点の名前
		uint32_t vertexHandle; ///< 頂点バッファのハンドル

		QFE_REFLECT_BEGIN(VertexComponent)
			QFE_REFLECT_MEMBER(vertexName)
		QFE_REFLECT_END()
	};
}