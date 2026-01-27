/**
 * @file QuickForgeEngine.cpp
 * @brief エンジンのエントリーポイント実装
 */

#include "QuickForgeEngine.h"
#include "engine/include/WindowsEngineCore.h"
#include "engine/include/core/IEngineCore.h"
#include <memory>

using namespace QFE;

/**
 * @brief Windows上でのエンジン実行
 * @param hInstance インスタンスハンドル
 * @param lpCmdLine コマンドライン引数
 */
void QuickForgeEngine::RunOnWindows(HINSTANCE& hInstance, LPSTR& lpCmdLine) {
	std::unique_ptr<IEngineCore> engineCore = std::make_unique<WindowsEngineCore>(hInstance, lpCmdLine);
	engineCore->Initialize();
	engineCore->MainLoop();
	engineCore->Shutdown();
}
