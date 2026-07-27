#pragma once

/// @namespace QFE::GRAPHIC
/// @brief 描画に使用する関数が定義されている名前空間
namespace QFE::GRAPHIC {
	/// @brief グラフィックエンジンのインターフェースクラス
	class IGraphicEngine {
	public:
		virtual ~IGraphicEngine() = default;

		/// @name システム必須関数
		/// @brief エンジン側で必ず特定の順序で呼び出す必要がある関数群
		/// @{

		virtual void Initialize() = 0;
		virtual void PreDraw() = 0;
		virtual void Draw() = 0;
		virtual void PostDraw() = 0;
		virtual void Shutdown() = 0;
	};
}