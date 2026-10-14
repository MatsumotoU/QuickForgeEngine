#pragma once
#include <memory>
#include <string>

// 使う機能一覧
namespace QFE::WINDOW {
	class IGameWindowManager;
}
namespace QFE::GRAPHIC {
	class IGraphicEngine;
}
namespace QFE::GUI {
	class IGuiManager;
}

/// @brief フレームワーク
namespace QFE::FRAMEWORK {
	/// @brief GUIマネージャを作る関数.グラフィックエンジンとウィンドウマネージャのポインタを引数に取ります.ウィンドウ名が空の場合はメインウィンドウのハンドルを取得します.
	std::unique_ptr<QFE::GUI::IGuiManager> CreateGuiManager(
		const QFE::GRAPHIC::IGraphicEngine* graphicEngine, const QFE::WINDOW::IGameWindowManager* windowManager, const std::string& windowName);

	/// @brief GUIマネージャを終了する関数.GUIマネージャのポインタを引数に取ります.
	bool ShutdownGui(QFE::GUI::IGuiManager* guiManager);
}