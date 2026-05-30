#include "GameWindow.h"
#include "string/MyString.h"
#include <cassert>

namespace QFE {

	void GameWindow::Initialize(const uint32_t& width, const uint32_t& height, const std::string& windowName) {
		configData_.clientWidth = width;
		configData_.clientHeight = height;
		configData_.windowName = Utf8ToUtf16(windowName);
		configData_.isCanDropFiles = true;

		eventManagerPtr_ = std::make_unique<WindowEventsManager>();
		WindowGenerator::CreateGameWindow(wc_, hwnd_, configData_, proc_, eventManagerPtr_.get());
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

	std::string GameWindow::GetWindowName() const {
		assert(!configData_.windowName.empty() && "Window name is empty");
		return ConvertString(configData_.windowName);
	}

	HWND GameWindow::GetHwnd() const {
		return hwnd_;
	}

}
