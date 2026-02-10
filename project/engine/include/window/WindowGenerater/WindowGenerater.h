#pragma once
#include <Windows.h>
#include <cstdint>
#include "Data/WindowConfigData.h"

namespace QFE {
	class WindowEventsManager;
	/// @brief ゲームウィンドウを生成するクラス
	class WindowGenerator {
	public:
		/// @brief ゲームウィンドウを生成する
		/// @param wc ウィンドウクラス(WinAPI)
		/// @param hwnd ウィンドウハンドル(WinAPI)
		/// @param config ウィンドウ設定データ
		/// @param proc ウィンドウプロシージャ(WinAPI)
		/// @param eventManager ウィンドウイベントマネージャー
		static void CreateGameWindow(WNDCLASS& wc, HWND& hwnd,WindowConfigData& config, WNDPROC& proc,WindowEventsManager* eventManager);
	};
}
