#pragma once

namespace QFE::GRAPHIC {
	enum class InternalApiType;
	class IGraphicEngine;
	struct FeatureState;
}

namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	bool CreateGraphicEngine(QFE::GRAPHIC::IGraphicEngine* engine, QFE::GRAPHIC::InternalApiType apiType);
	/// @brief DirectX12グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	bool CreateD3D12GraphicEngine(QFE::GRAPHIC::IGraphicEngine* engine, QFE::GRAPHIC::FeatureState featureState);
}