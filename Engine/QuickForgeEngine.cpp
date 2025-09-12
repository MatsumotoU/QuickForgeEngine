#include "QuickForgeEngine.h"
#include "Platform/Windows/WindowsEngineCore.h"
#include <memory>

void QuickForgeEngine::RunOnWindows(HINSTANCE& hInstance, LPSTR& lpCmdLine) {
	std::unique_ptr<IEngineCore> engineCore = std::make_unique<WindowsEngineCore>(hInstance, lpCmdLine);
	engineCore->Initialize();
	engineCore->MainLoop();
	engineCore->Shutdown();
}
