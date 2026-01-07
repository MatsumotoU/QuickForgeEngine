#include "engine/QuickForgeEngine.h"
#include "engine/include/WindowsEngineCore.h"
#include "engine/include/core/IEngineCore.h"
#include <memory>
#include <exception>
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include <format>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hPrevInstance; nCmdShow;

    std::unique_ptr<IEngineCore> engineCore = std::make_unique<WindowsEngineCore>(hInstance, lpCmdLine);

    try {
        engineCore->Initialize();
        engineCore->MainLoop();
    }
    catch (const std::exception& e) {
        DebugLog(std::format("An exception occurred: {}\n", e.what()));
    }
    catch (...) {
        DebugLog("An unknown exception occurred.\n");
    }

    engineCore->Shutdown();

    return 0;
}
