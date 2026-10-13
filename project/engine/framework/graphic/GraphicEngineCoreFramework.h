#pragma once
#include "info/SupportedOs.h"
#include <memory>

namespace QFE::WINDOW {
	class IGameWindowManager;
}

namespace QFE::GRAPHIC {
	class IGraphicEngine;
	struct FeatureState;
}

namespace QFE::FRAMEWORK {
	/// @brief グラフィックエンジンを生成する関数.ウィンドウのハンドルを引数に取ります.
	std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> CreateGraphicEngine(
		const QFE::WINDOW::IGameWindowManager& windowManager,
		QFE::INFO::SupportedOs supportedOs, QFE::GRAPHIC::FeatureState featureState);
}
