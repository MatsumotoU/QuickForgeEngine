#pragma once
#include "framework/graphic/GraphicEngineFeatureState.h"
#include <memory>

namespace QFE::WINDOW {
	class IGameWindowManager;
}

namespace QFE::GRAPHIC {
	class IGraphicEngine;
}

namespace QFE::FRAMEWORK {

	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> CreateGraphicEngine(
		const QFE::WINDOW::IGameWindowManager& windowManager,
		const QFE::FRAMEWORK::GraphicEngineFeatureState& featureState);

	/// @brief 指定したグラフィック機能が初期化済みで利用可能かを取得します。
	[[nodiscard]] bool IsGraphicEngineFeatureAvailable(
		const QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::FRAMEWORK::GraphicEngineFeature feature);

	/// @brief グラフィックエンジンで描画する前の処理を行う関数
	bool PreDrawGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンで描画した後の処理を行う関数
	bool PostDrawGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine);
	/// @brief 深度ステンシルを使用せずにスワップチェーンを描画先へ設定する関数
	bool SetSwapChainRenderTarget(QFE::GRAPHIC::IGraphicEngine& graphicEngine);
	/// @brief グラフィックエンジンを終了する関数
	bool ShutdownGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine);
}
