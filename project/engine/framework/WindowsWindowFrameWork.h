#pragma once
#include "window/GameWindowManager.h"
#include "window/WindowsUtils.h"

namespace QFE::FRAMEWORK {
	/// WindowManagerを作りながらメインウィンドウを表示する
	std::unique_ptr<QFE::GameWindowManager> CreateWindowManager(
		const std::string& mainWindowName, uint32_t width, uint32_t height);
}
