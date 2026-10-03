#include "WindowCoreFrameWork.h"
#include "window/GameWindowManager.h"
using namespace QFE::FRAMEWORK;

std::unique_ptr<QFE::WINDOW::IGameWindowManager> QFE::FRAMEWORK::CreateWindowManager(
	const std::string& mainWindowName, uint32_t width, uint32_t height){
	std::unique_ptr<QFE::WINDOW::IGameWindowManager> windowManager = std::make_unique<QFE::WINDOW::IGameWindowManager>();
	windowManager->Initialize();
	windowManager->AddWindow(width, height, mainWindowName);
	return windowManager;
}
