#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>

#include <filesystem>
#include <string_view>

#include "ProjectGeneratorSystemCore.h"
#include "graphics/interface/DirectXRenderFramework.h"
#include "graphics/interface/DirectXResourceFramework.h"

#include "ProjectGenerator.h"

struct WindowsGuiToolSystems {
	WindowsGuiToolSystems() = default;
	WindowsGuiToolSystems(const WindowsGuiToolSystems&) = delete;
	WindowsGuiToolSystems& operator=(const WindowsGuiToolSystems&) = delete;
	WindowsGuiToolSystems(WindowsGuiToolSystems&&) = delete;
	WindowsGuiToolSystems& operator=(WindowsGuiToolSystems&&) = delete;

	std::unique_ptr<QFE::WINDOW::IGameWindowManager> windowManager;
	std::unique_ptr<QFE::GRAPHIC::IGraphicEngine> graphicEngine;
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager;
};

/// @brief アプリケーションのエントリーポイント
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
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

	// エンジンの機能生成
	QFE::APPLICATION::ProjectGeneratorSystemCore engineSystems;
	engineSystems.windowManager = QFE::FRAMEWORK::CreateWindowManager(
		"Project Generator", 1280, 720);
	engineSystems.graphicEngine = QFE::FRAMEWORK::CreateGraphicEngine(
		*engineSystems.windowManager,
		QFE::INFO::SupportedOs::Windows,
		QFE::GRAPHIC::FeatureState());
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
		if (QFE::GRAPHIC::GetDepthStencilResourceHandle(
			engineSystems.graphicEngine.get(),
			depthStencilHandle)) {
			QFE::GRAPHIC::SetRenderTarget(
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
