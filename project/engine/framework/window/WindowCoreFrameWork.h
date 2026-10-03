#pragma once
#include <memory>
#include <string>
#include <cstdint>

namespace QFE::WINDOW {
	class IGameWindowManager;
}

namespace QFE::FRAMEWORK {
	/// @brief WindowManagerを作りながらメインウィンドウを表示する
	std::unique_ptr<QFE::WINDOW::IGameWindowManager> CreateWindowManager(
		const std::string& mainWindowName, uint32_t width, uint32_t height);
}
