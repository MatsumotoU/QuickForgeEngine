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
	/// @brief GUIマネージャを作る関数.グラフィックエンジンとウィンドウマネージャを引数に取ります.ウィンドウ名が空の場合はメインウィンドウのハンドルを取得します.
	std::unique_ptr<QFE::GUI::IGuiManager> CreateGuiManager(
		const QFE::GRAPHIC::IGraphicEngine& graphicEngine, const QFE::WINDOW::IGameWindowManager& windowManager, const std::string& windowName = "");

	/// @brief GUIマネージャを終了する関数.GUIマネージャを引数に取ります.
	bool ShutdownGui(QFE::GUI::IGuiManager& guiManager);

	/// @brief 描画前の処理を行う関数.GUIマネージャを引数に取ります.
	/// @param guiManager
	void PreDrawGui(QFE::GUI::IGuiManager& guiManager);
	/// @brief 描画後の処理を行う関数.GUIマネージャを引数に取ります.
	/// @param guiManager
	void PostDrawGui(QFE::GUI::IGuiManager& guiManager);
}
