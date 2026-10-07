#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#include <filesystem>
#include <string_view>

#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/window/WindowsWindowFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "framework/graphic/GraphicContext.h"

#include "window/GameWindowManager.h"
#include "gui/D3D12GuiManager.h"
#include "scene/SceneManager.h"
#include "components/AllComponent.h"
#include "core/logger/MyDebugLog.h"
#include "core/string/MyString.h"
#include "core/timer/FPSCounter.h"
#include "assetfactory/model/AssimpModelLoader.h"
#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

#include "ProjectGenerator.h"

struct WindowsGuiToolSystems {
	std::unique_ptr<QFE::GameWindowManager> windowManager;
	std::unique_ptr<QFE::FRAMEWORK::GraphicContext> graphicEngine;
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager;
};

/// @brief Windows project generator entry point.
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	int argumentCount = 0;
	LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
	if (arguments != nullptr && argumentCount >= 3 &&
		std::wstring_view(arguments[1]) == L"--generate-config") {
		QFE::APPLICATION::ProjectGenerator batchGenerator;
		const bool generated = batchGenerator.GenerateFromConfiguration(
			std::filesystem::path(arguments[2]));
		LocalFree(arguments);
		return generated ? 0 : 1;
	}
	if (arguments != nullptr) {
		LocalFree(arguments);
	}

	// Store engine resources.
	WindowsGuiToolSystems engineSystems;
	// Create engine systems.
	engineSystems.windowManager =
		QFE::FRAMEWORK::CreateWindowManager(
			"Project Generator", 1280, 720);
	// Get the window handle.
	HWND hwnd = QFE::FRAMEWORK::GetWindowHandle(
		engineSystems.windowManager.get(),
		"Project Generator");
	// Create the graphics engine.
	engineSystems.graphicEngine =
		QFE::FRAMEWORK::CreateGraphicEngine(hwnd);
	// Create the GUI manager.
	engineSystems.guiManager =
		QFE::FRAMEWORK::CreateGuiManager(
			engineSystems.graphicEngine.get(), hwnd);


	QFE::APPLICATION::ProjectGenerator projectGenerator;
	QFE::APPLICATION::ImGuiContext imguiContext;
	imguiContext.hwnd = hwnd;
	projectGenerator.Initialize(imguiContext);

	// Main loop.
	while (QFE::FRAMEWORK::IsMainWindowActive(
		engineSystems.windowManager.get())) {

		if (!QFE::FRAMEWORK::ProcessWindowsApplicationMessage()) {
			break;
		}

		QFE::FRAMEWORK::PreDrawGraphicEngine(
			engineSystems.graphicEngine.get());

		engineSystems.guiManager->PreDraw();

		projectGenerator.Update();
		projectGenerator.Draw();

		QFE::GRAPHIC::DirectXResourceHandle depthStencilHandle;
		if (QFE::FRAMEWORK::GetDepthStencilResourceHandle(
			engineSystems.graphicEngine.get(),
			depthStencilHandle)) {
			QFE::FRAMEWORK::SetRenderTarget(
				engineSystems.graphicEngine.get(),
				depthStencilHandle,
				{ QFE::GRAPHIC::RenderTargetHandle::SwapChain });
		}

		engineSystems.guiManager->PostDraw();
		QFE::FRAMEWORK::PostDrawGraphicEngine(
			engineSystems.graphicEngine.get());
	}

	// Shutdown.
	projectGenerator.Shutdown();
	QFE::FRAMEWORK::ShutdownGui(
		engineSystems.guiManager.get());
	QFE::FRAMEWORK::ShutdownGraphicEngine(
		engineSystems.graphicEngine.get());
	return 0;
}
