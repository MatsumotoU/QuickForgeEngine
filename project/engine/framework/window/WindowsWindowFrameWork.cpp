#include "WindowsWindowFrameWork.h"

std::unique_ptr<QFE::GameWindowManager> QFE::FRAMEWORK::CreateWindowManager(
	const std::string& mainWindowName, uint32_t width, uint32_t height) {

	std::unique_ptr<QFE::GameWindowManager> windowManger = std::make_unique<QFE::GameWindowManager>();
	windowManger->Initialize();
	windowManger->AddWindow(width, height, mainWindowName);
	return windowManger;
}