/**
 * @file EngineGlobalValue.h
 * @brief エンジン全体で共有されるグローバル変数の定義
 */

#pragma once
#include <stdint.h>
#include <string>

/**
 * @namespace QFE::EngineGlobalValue
 * @brief エンジンの実行状態や設定を保持する名前空間
 */
namespace QFE::EngineGlobalValue {
	/// @brief ウィンドウの横幅
	extern uint32_t windowWidth;
	/// @brief ウィンドウの縦幅
	extern uint32_t windowHeight;
	/// @brief 前フレームからの経過時間（秒）
	extern float deltaTime;
	/// @brief 現在のフレームレート（FPS）
	extern float fps;
}
