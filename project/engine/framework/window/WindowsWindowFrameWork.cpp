#include "WindowsWindowFrameWork.h"
#include "window/GameWindowManager.h"
#include "window/WindowsUtils.h"

#include "EngineDefines.h"

std::unique_ptr<QFE::GameWindowManager> QFE::FRAMEWORK::CreateWindowManager(
	const std::string& mainWindowName, uint32_t width, uint32_t height) {

	std::unique_ptr<QFE::GameWindowManager> windowManger = std::make_unique<QFE::GameWindowManager>();
	windowManger->Initialize();
	windowManger->AddWindow(width, height, mainWindowName);
	return windowManger;
}

HWND QFE::FRAMEWORK::GetWindowHandle(const GameWindowManager* windowManager, const std::string& windowName) {
	if(windowManager == nullptr) {
		QFE_LOG("windowManager is null");
		return nullptr;
	}
	return windowManager->GetWindow(windowName);
}