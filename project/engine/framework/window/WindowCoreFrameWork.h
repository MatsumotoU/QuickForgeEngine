#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include "window/interface/IGameWindowManager.h"

namespace QFE::FRAMEWORK {
	/// @brief WindowManagerを作りながらメインウィンドウを表示する
	std::unique_ptr<QFE::WINDOW::IGameWindowManager> CreateWindowManager(
		const std::string& mainWindowName, uint32_t width, uint32_t height);
	/// @brief 終了処理を実行する。ウィンドウマネージャーの所有権は呼び出し元に残る。
	void ShutdownWindowManager(QFE::WINDOW::IGameWindowManager& windowManager);
	/// @brief 未処理のドロップを取得し、キューから取り除く。
	/// @details 戻り値はパスとスクリーン座標のデータで、Windows 固有のハンドルを含まない。
	std::vector<QFE::WINDOW::DroppedFile> ConsumeDroppedFiles(QFE::WINDOW::IGameWindowManager& windowManager);
	/// @brief メインウィンドウがアクティブかどうかを返す
	bool IsMainWindowActive(const QFE::WINDOW::IGameWindowManager& windowManager);
	/// @brief ウィンドウのメッセージを処理する
	bool ProcessWindowsApplicationMessage();
	/// @brief メインウィンドウの状態を確認しながら、ウィンドウのメッセージを処理する
	bool CanLoopWindowManager(const QFE::WINDOW::IGameWindowManager& windowManager);
}
