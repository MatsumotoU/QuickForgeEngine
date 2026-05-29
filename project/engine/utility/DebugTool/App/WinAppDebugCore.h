/**
 * @file WinAppDebugCore.h
 * @brief アプリケーションのデバッグ機能を管理するクラス
 */

#pragma once
#include "CrashHandler.h"

namespace QFE {
	/**
	 * @class WinAppDebugCore
	 * @brief クラッシュハンドラの初期化など、Windowsアプリとしてのデバッグ基盤を管理するクラス
	 */
	class WinAppDebugCore final {
	public:
		/**
		 * @brief コンストラクタ
		 * @param lpCmdLine コマンドライン引数
		 */
		WinAppDebugCore(const LPSTR& lpCmdLine);
		~WinAppDebugCore();
	};
}