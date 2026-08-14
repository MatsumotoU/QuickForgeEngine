#pragma once
#include "math/MathInclude.h"
#include "EngineDefines.h"

namespace QFE::SCENE {
	/// @brief オブジェクトの情報を表すコンポーネントです。
	struct ModelRenderComponent {
		std::string modelName = "Primitive/Box"; ///< モデルまたは組み込みプリミティブの名前
		std::string textureName; ///< 空ならモデル内蔵、内蔵もなければWhite1x1

		bool canRender = false; ///< レンダリング可能かどうかのフラグ

		uint32_t vertexResourceHandle = 0; ///< 頂点リソースハンドル
		uint32_t indexResourceHandle = 0; ///< インデックスリソースハンドル
		uint32_t transformMatrixBufferHandle = 0; ///< ワールド行列バッファハンドル
		uint32_t materialResourceHandle = 0; ///< マテリアルリソースハンドル
		uint32_t textureResourceHandle = 0; ///< テクスチャリソースハンドル

		std::string renderErrorMessage; ///< レンダリングエラーメッセージ

		QFE_REFLECT_BEGIN(ModelRenderComponent)
			QFE_REFLECT_MEMBER(modelName)
			QFE_REFLECT_MEMBER(textureName)
			QFE_REFLECT_END()
	};
	QFE_COMPONENT(ModelRenderComponent)
}
