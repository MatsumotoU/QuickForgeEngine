#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief オブジェクトの情報を表すコンポーネントです。
	struct ModelRenderComponent {
		std::string modelName; ///< モデルの名前

		bool canRender; ///< レンダリング可能かどうかのフラグ

		uint32_t vertexResourceHandle; ///< 頂点リソースハンドル
		uint32_t transformMatrixBufferHandle; ///< ワールド行列バッファハンドル
		uint32_t materialResourceHandle; ///< マテリアルリソースハンドル
		uint32_t textureResourceHandle; ///< テクスチャリソースハンドル

		QFE_REFLECT_BEGIN(ModelRenderComponent)
			QFE_REFLECT_MEMBER(modelName)
		QFE_REFLECT_END()
	};
}