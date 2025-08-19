#pragma once
#include <Windows.h>
#include <memory>
#include "Core/Window/IGameWindow.h"
#include "WindowGenerater/WindowGenerater.h"
#include "WindowEventsManager/WindowEventsManager.h"

class GameWindow final:public IGameWindow{
public:
	void Initialize(const uint32_t& width, const uint32_t& height, const std::string& windowName)override;
	void Update()override;
	void Draw()override;
	void Shutdown()override;
	bool IsWindowActive() const override;

private:
	HWND hwnd_ = nullptr;
	WNDCLASS wc_ = {};
	WindowConfigData configData_;
	WNDPROC proc_ = WindowEventsManager::WindowProc;
	std::unique_ptr<WindowEventsManager> eventManagerPtr_;
};