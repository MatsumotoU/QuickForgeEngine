/**
 * @file main.cpp
 * @brief エディタアプリケーションのエントリポイント
 */

#include "engine/QuickForgeEngine.h"
#include "engine/include/WindowsEngineCore.h"
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"

#include "editor/include/OnWindowsEditor.h"

#include <memory>
#include <exception>
#include <format>

using namespace QFE;

/**
 * @brief Windowsアプリケーションのエントリポイント
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hPrevInstance; nCmdShow;

    // エンジンコアの生成（Windowsの実装を使用）
    std::unique_ptr<QFE::IEngineCore> engineCore = std::make_unique<QFE::WindowsEngineCore>(hInstance, lpCmdLine);

    try {
		// エンジンの初期化とメインループの開始
        engineCore->Initialize(std::make_unique<OnWindowsEditor>());
        engineCore->MainLoop();
    }
    catch (const std::exception& e) {
        e;
#ifdef QFE_OPTIMIZE_OFF
        DebugLog(std::format("An exception occurred: {}\n", e.what()));
#endif // _DEBUG
    }

	// エンジンの終了処理
    engineCore->Shutdown();

    return 0;
}
