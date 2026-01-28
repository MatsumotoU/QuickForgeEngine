/**
 * @file QuickForgeEngine.h
 * @brief エンジンのエントリポイントを定義するヘッダー
 */

#pragma once
#include <Windows.h>

/**
 * @namespace QuickForgeEngine
 * @brief エンジンのエントリポイントを含む名前空間
 */
namespace QuickForgeEngine {
    /**
     * @brief Windows上でエンジンを実行する
     * @param hInstance インスタンスハンドル
     * @param lpCmdLine コマンドライン引数
     */
	void RunOnWindows(HINSTANCE& hInstance, LPSTR& lpCmdLine);
}
