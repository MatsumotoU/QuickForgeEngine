#pragma once
#include <memory>

namespace QFE::GRAPHIC {
	enum class InternalApiType;
	class IGraphicEngine;
	struct FeatureState;
}

namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	bool CreateGraphicEngine(QFE::GRAPHIC::IGraphicEngine* engine, QFE::GRAPHIC::InternalApiType apiType);

	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> CreateGraphicEngine(QFE::GRAPHIC::InternalApiType apiType);
}