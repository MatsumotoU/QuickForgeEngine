#pragma once
#include <memory>
#include <string>
#include <cstdint>

#define NOMINMAX
#include <Windows.h>

namespace QFE {
	class GameWindowManager;
}

namespace QFE::FRAMEWORK {
	///  @brief WindowManagerを作りながらメインウィンドウを表示する
	std::unique_ptr<QFE::GameWindowManager> CreateWindowManager(
		const std::string& mainWindowName, uint32_t width, uint32_t height);

	/// @brief WindowManagerからウィンドウハンドルを取得する
	HWND GetWindowHandle(const GameWindowManager* windowManager, const std::string& windowName);
}
