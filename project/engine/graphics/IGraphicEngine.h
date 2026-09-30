#pragma once
#include "InternalApiType.h"
#include "FeatureState.h"

/// @namespace QFE::GRAPHIC
/// @brief 描画に使用する関数が定義されている名前空間
namespace QFE::GRAPHIC {
	/// @brief グラフィックエンジンのインターフェースクラス
	class IGraphicEngine {
	public:
		virtual ~IGraphicEngine() = default;

		virtual void Initialize() = 0;
		virtual void PreDraw() = 0;
		virtual void Draw() = 0;
		virtual void PostDraw() = 0;
		virtual void Shutdown() = 0;

		/// @brief 内部グラフィックAPIの種類を取得する関数
		virtual QFE::GRAPHIC::InternalApiType GetInternalApiType() const = 0;
		/// @brief グラフィックエンジンの機能が有効かどうかを取得する関数
		virtual QFE::GRAPHIC::FeatureState GetFeatureState() const = 0;
	};
}