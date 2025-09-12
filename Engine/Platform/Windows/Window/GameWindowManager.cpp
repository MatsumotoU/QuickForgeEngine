#include "GameWindowManager.h"
#include "GameWindow.h"

#ifdef _DEBUG

#endif // _DEBUG

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
			// IGameWindow* から GameWindow* へキャスト
			if (auto gameWindow = dynamic_cast<GameWindow*>(window.get())) {
				return gameWindow->GetHwnd();
			}
		}
	}
	throw std::runtime_error("指定されたウィンドウ名が見つかりませんでした。");

}
