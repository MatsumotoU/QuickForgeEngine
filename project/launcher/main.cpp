#define NOMINMAX
#include <Windows.h>

#include "window/GameWindowManager.h"
#include "window/WindowsUtils.h"
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "gui/D3D12GuiManager.h"
#include "camera/CameraManager.h"
#include "core/logger/MyDebugLog.h"
#include "core/process/ProcessUtil.h"

#include "assetfactory/model/AssimpModelLoader.h"

#include "core/math/transform/Transform.h"

#include <imgui/imgui.h>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	std::string windowName = "Launcher Window";
	std::unique_ptr<QFE::GameWindowManager> gameWindowManager = std::make_unique<QFE::GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(1280, 720, windowName);

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine =
		QFE::FRAMEWORK::CreateGraphicEngine(gameWindowManager->GetWindow(windowName));
	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager =
		QFE::FRAMEWORK::CreateGuiManager(graphicEngine.get(), gameWindowManager->GetWindow(windowName));

	// カメラマネージャの初期化とカメラの作成
	QFE::CAMERA::CameraManager cameraManager;
	cameraManager.Initialize();
	QFE::CAMERA::CameraHandle cameraHandle = cameraManager.CreateCamera(0.0f, 1280.0f, 0.0f, 720.0f, 0.1f, 100.0f, 0.45f);

	// メインループ
	while (gameWindowManager->IsWindowActive()) {
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			// WM_QUITメッセージが来たらループを抜ける
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			graphicEngine->PreDraw();
			guiManager->PreDraw();

			ImGui::Begin("Test Window");
			if (ImGui::Button("Launch Runtime")) {
				std::wstring selectedFilePath;
				if (QFE::WINDOW::RequestGetFilePathFromUser(
					gameWindowManager->GetWindow(windowName),
					L"Exe Files", L"*.exe",
					selectedFilePath)) {
				}
				QFE::ProcessUtil::LaunchExe(QFE::ConvertString(selectedFilePath), "");
			}
			ImGui::End();

			graphicEngine->SetRenderTarget(QFE::GRAPHIC::RenderTargetHandle::SwapChain);
			guiManager->PostDraw();
			graphicEngine->PostDraw();
		}
	}

	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
	return 0;
}
