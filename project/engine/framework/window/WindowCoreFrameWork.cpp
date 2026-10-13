#include "WindowCoreFrameWork.h"
#include "window/GameWindowManager.h"
#include <unordered_map>
#include <functional>
using namespace QFE::FRAMEWORK;

namespace {
	/// @brief Windows用のウィンドウマネージャを作成する
	std::unique_ptr<QFE::WINDOW::IGameWindowManager> CreateWindowsWindowManager(uint32_t width, uint32_t height, const std::string& mainWindowName) {
		std::unique_ptr<QFE::WINDOW::IGameWindowManager> windowManager = std::make_unique<QFE::WINDOW::GameWindowManager>();
		windowManager->Initialize();
		windowManager->AddWindow(width, height, mainWindowName);
		return std::move(windowManager);
	}

	/// @brief それぞれのOSのウィンドウファクトリマップ
	std::unordered_map<QFE::INFO::SupportedOs, std::function<std::unique_ptr<QFE::WINDOW::IGameWindowManager>(uint32_t, uint32_t, const std::string&)>> windowManagerFactory = {
		{ QFE::INFO::SupportedOs::Windows, CreateWindowsWindowManager}
	};
}

std::unique_ptr<QFE::WINDOW::IGameWindowManager> QFE::FRAMEWORK::CreateWindowManager(
	const std::string& mainWindowName, uint32_t width, uint32_t height, QFE::INFO::SupportedOs supportedOs) {
	std::unique_ptr<QFE::WINDOW::IGameWindowManager> windowManager = nullptr;
	windowManager = windowManagerFactory[supportedOs](width, height, mainWindowName);
	return std::move(windowManager);
}

bool QFE::FRAMEWORK::IsMainWindowActive(const std::unique_ptr<QFE::WINDOW::IGameWindowManager>& windowManager) {

	return false;
}
