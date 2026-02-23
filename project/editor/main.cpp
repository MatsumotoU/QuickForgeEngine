/**
 * @file main.cpp
 * @brief エディタアプリケーションのエントリポイント
 */

#include "engine/QuickForgeEngine.h"
#include "engine/include/WindowsEngineCore.h"
#include "engine/include/core/IEngineCore.h"
#include <memory>
#include <exception>
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
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
        engineCore->Initialize();
        engineCore->MainLoop();
    }
    catch (const std::exception& e) {
        e;
#ifdef QFE_OPTIMIZE_OFF
        DebugLog(std::format("An exception occurred: {}\n", e.what()));
#endif // _DEBUG
    }

    engineCore->Shutdown();

    return 0;
}
