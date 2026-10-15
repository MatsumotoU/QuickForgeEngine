/// @file WindowCoreFrameWork.cpp
/// @brief Windows向けのウィンドウフレームワークの実装

#include "framework/window/WindowCoreFrameWork.h"
#include "window/GameWindowManager.h"
#include "EngineDefines.h"
#define NOMINMAX
#include <Windows.h>
using namespace QFE::FRAMEWORK;

std::unique_ptr<QFE::WINDOW::IGameWindowManager> QFE::FRAMEWORK::CreateWindowManager(
	const std::string& mainWindowName, uint32_t width, uint32_t height) {
	auto windowManager = std::make_unique<QFE::WINDOW::GameWindowManager>();
	windowManager->Initialize();
	windowManager->AddWindow(width, height, mainWindowName);
	return windowManager;
}

bool QFE::FRAMEWORK::IsMainWindowActive(const QFE::WINDOW::IGameWindowManager& windowManager) {
	return windowManager.IsWindowActive();
}

bool QFE::FRAMEWORK::ProcessWindowsApplicationMessage() {
	// メッセージを処理
	MSG msg{};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

bool QFE::FRAMEWORK::CanLoopWindowManager(const QFE::WINDOW::IGameWindowManager& windowManager)
{
	return QFE::FRAMEWORK::IsMainWindowActive(windowManager) && QFE::FRAMEWORK::ProcessWindowsApplicationMessage();
}
