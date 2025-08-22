#include "GameWindow.h"
#include "String/MyString.h"

void GameWindow::Initialize(const uint32_t& width, const  uint32_t& height, const  std::string& windowName) {
	configData_.clientWidth = width;
	configData_.clientHeight = height;
	configData_.windowName = std::wstring(windowName.begin(), windowName.end());
	configData_.isCanDropFiles = true; // ドラッグ＆ドロップを有効にする

	eventManagerPtr_ = std::make_unique<WindowEventsManager>();
	WindowGenerater::CreateGameWindow(wc_, hwnd_, configData_, proc_, eventManagerPtr_.get());
}

void GameWindow::Update() {
}

void GameWindow::Draw() {
}

void GameWindow::Shutdown() {
}

bool GameWindow::IsWindowActive() const {
	if (hwnd_ && IsWindow(hwnd_)) {
		return true;
	}
	return false;
}
