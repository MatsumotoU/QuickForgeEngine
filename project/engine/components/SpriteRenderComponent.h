#pragma once

#include "EngineDefines.h"

#include <cstdint>
#include <string>

namespace QFE::SCENE {
	/// @brief 画面座標で描画するスプライトの情報を表すコンポーネントです。
	struct SpriteRenderComponent {
		std::string textureName; ///< resources/ から読み込むテクスチャ名
		int32_t drawOrder = 0; ///< 小さい値から先に描画
		bool visible = true; ///< スプライトを描画するかどうか

		bool canRender = false; ///< 現在のフレームで描画可能かどうか
		uint32_t transformMatrixBufferHandle = 0; ///< WVP行列バッファハンドル
		uint32_t materialResourceHandle = 0; ///< マテリアルリソースハンドル
		uint32_t textureResourceHandle = 0; ///< テクスチャリソースハンドル
		std::string renderErrorMessage; ///< 描画できない理由

		QFE_REFLECT_BEGIN(SpriteRenderComponent)
			QFE_REFLECT_MEMBER(textureName)
			QFE_REFLECT_MEMBER(drawOrder)
			QFE_REFLECT_MEMBER(visible)
		QFE_REFLECT_END()
	};

	QFE_COMPONENT(SpriteRenderComponent)
}
