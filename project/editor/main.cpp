#include "engine/QuickForgeEngine.h"
#include "engine/include/WindowsEngineCore.h"
#include "engine/include/core/IEngineCore.h"
#include <memory>
#include <exception>
#include "engine/include/utility/DebugTool/DebugLog/MyDebugLog.h"
#include <format>
#include <filesystem>
#include <stdexcept>

namespace {
void SetResourceWorkingDirectory() {
    std::wstring executablePath(MAX_PATH, L'\0');
    DWORD pathLength = 0;
    for (;;) {
        pathLength = GetModuleFileNameW(nullptr, executablePath.data(), static_cast<DWORD>(executablePath.size()));
        if (pathLength == 0) {
            throw std::runtime_error("Could not determine the executable path");
        }
        if (pathLength < executablePath.size() - 1) {
            break;
        }
        executablePath.resize(executablePath.size() * 2);
    }
    executablePath.resize(pathLength);

    // Support both a packaged executable beside Resources and the generated/outputs build layout.
    for (auto directory = std::filesystem::path(executablePath).parent_path();; directory = directory.parent_path()) {
        for (const auto& root : {directory, directory / L"project"}) {
            if (std::filesystem::exists(root / L"Resources" / L"Config" / L"SceneConfig.json")) {
                std::filesystem::current_path(root);
                return;
            }
        }
        if (directory == directory.root_path()) {
            break;
        }
    }
    throw std::runtime_error("Could not find Resources/Config/SceneConfig.json near the executable");
}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    hPrevInstance; nCmdShow;

    std::unique_ptr<IEngineCore> engineCore;

    try {
        SetResourceWorkingDirectory();
        engineCore = std::make_unique<WindowsEngineCore>(hInstance, lpCmdLine);
        engineCore->Initialize();
        engineCore->MainLoop();
    }
    catch (const std::exception& e) {
        DebugLog(std::format("An exception occurred: {}\n", e.what()));
    }
    catch (...) {
        DebugLog("An unknown exception occurred.\n");
    }

    if (engineCore) {
        engineCore->Shutdown();
    }

    return 0;
}
