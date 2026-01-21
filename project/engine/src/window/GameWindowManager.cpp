#include "engine/include/window/GameWindowManager.h"
#include "engine/include/window/GameWindow.h"

#ifdef QFE_OPTIMIZE_OFF

#endif // QFE_OPTIMIZE_OFF

GameWindowManager::GameWindowManager() {
}

void GameWindowManager::Initialize() {
	
}

void GameWindowManager::Update() {
	for (auto& window : windows) {
		window->Update();
	}
}

void GameWindowManager::Draw() {
	for (auto& window : windows) {
		window->Draw();
	}
}

void GameWindowManager::Shutdown() {
	for (auto& window : windows) {
		window->Shutdown();
	}
	windows.clear();
}

void GameWindowManager::AddWindow(const uint32_t& width, const uint32_t& height, const std::string& windowName) {
	auto window = std::make_unique<GameWindow>();
	window->Initialize(width, height, windowName);
	windows.push_back(std::move(window));
}

bool GameWindowManager::IsWindowActive() const {
	for (const auto& window : windows) {
		if (window->IsWindowActive()) {
			return true;
		}
	}
	
	return false;
}

HWND GameWindowManager::GetWindow(const std::string windowName) const {
	for (const auto& window : windows) {
		if (window->GetWindowName() == windowName) {
			// IGameWindow* 縺九ｉ GameWindow* 縺ｸ繧ｭ繝｣繧ｹ繝・
			if (auto gameWindow = dynamic_cast<GameWindow*>(window.get())) {
				return gameWindow->GetHwnd();
			}
		}
	}
	throw std::runtime_error("謖・ｮ壹＆繧後◆繧ｦ繧｣繝ｳ繝峨え蜷阪′隕九▽縺九ｊ縺ｾ縺帙ｓ縺ｧ縺励◆縲・+ windowName");

}
