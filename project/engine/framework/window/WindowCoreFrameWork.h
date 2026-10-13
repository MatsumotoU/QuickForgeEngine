#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include "info/SupportedOs.h"

namespace QFE::WINDOW {
	class IGameWindowManager;
}

namespace QFE::FRAMEWORK {
	/// @brief WindowManagerを作りながらメインウィンドウを表示する
	std::unique_ptr<QFE::WINDOW::IGameWindowManager> CreateWindowManager(
		const std::string& mainWindowName, uint32_t width, uint32_t height,QFE::INFO::SupportedOs supportedOs);
	/// @brief メインウィンドウがアクティブかどうかを返す
	bool IsMainWindowActive(const std::unique_ptr<QFE::WINDOW::IGameWindowManager>& windowManager);
}
