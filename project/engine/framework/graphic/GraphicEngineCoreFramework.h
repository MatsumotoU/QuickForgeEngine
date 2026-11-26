#pragma once
#include "framework/graphic/GraphicEngineFeatureState.h"
#include <cstdint>
#include <memory>

namespace QFE::WINDOW {
	class IGameWindowManager;
}

namespace QFE::GRAPHIC {
	class IGraphicEngine;
	enum class RenderTargetHandle : uint32_t;
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

	/// @brief エディターに表示するシーン描画先と GUI 用のテクスチャ ID を作る。
	/// @details 描画先のハンドルは共有データとして返し、API 固有のリソース操作は実装側で行う。
	/// @return 作成できた場合は true。失敗時は呼び出し元で初期化を中止する。
	bool CreateEditorSceneTexture(
		QFE::GRAPHIC::IGraphicEngine* graphicEngine,
		QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
		uintptr_t& outGuiTextureId,
		uint32_t width, uint32_t height);

	/// @brief グラフィックエンジンで描画する前の処理を行う関数
	bool PreDrawGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine);
	/// @brief グラフィックエンジンで描画した後の処理を行う関数
	bool PostDrawGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine);
	/// @brief 深度ステンシルを使用せずにスワップチェーンを描画先へ設定する関数
	bool SetSwapChainRenderTarget(QFE::GRAPHIC::IGraphicEngine& graphicEngine);
	/// @brief グラフィックエンジンを終了する関数
	bool ShutdownGraphicEngine(QFE::GRAPHIC::IGraphicEngine* graphicEngine);
}
