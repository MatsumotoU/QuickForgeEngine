#pragma once
#define NOMINMAX
#include <Windows.h>

#include <string>
#include <cstdint>

namespace QFE::FRAMEWORK {
	/// @brief QuickForgeのセットアップに必要な情報を格納する構造体
	struct QuickForgeSetupConfig {
		std::string mainWindowName; ///< ウィンドウの名前
		uint32_t mainWindowWidth;       ///< ウィンドウの幅
		uint32_t mainWindowHeight;      ///< ウィンドウの高さ
		HINSTANCE hInstance; ///< ウィンドウのインスタンスハンドル
	};
}