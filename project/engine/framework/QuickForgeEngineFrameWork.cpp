#include "QuickForgeEngineFrameWork.h"

bool QFE::FRAMEWORK::InitializeEngine(WindowsEngineSystems& windowsEngineSystems, const EngineConfigDesc& config) {
    // window生成
    windowsEngineSystems.windowManager =
        CreateWindowManager(config.mainWindowName, config.mainWindowWidth, config.mainWindowHeight);
    // window生成確認
    if (!windowsEngineSystems.windowManager) { return false; }
    // graphicEngine生成
    windowsEngineSystems.graphicEngine =
        CreateGraphicEngine(windowsEngineSystems.windowManager->GetWindow(config.mainWindowName));
    // graphicEngine確認
    if (!windowsEngineSystems.graphicEngine) { return false; }

    return true;
}

bool QFE::FRAMEWORK::FinalizeEngine(WindowsEngineSystems& windowsEngineSystems) {
    // graphic
    windowsEngineSystems.graphicEngine->Shutdown();
    // window
    windowsEngineSystems.windowManager->Shutdown();

    return true;
}
