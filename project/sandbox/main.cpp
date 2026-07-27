#define NOMINMAX
#include <Windows.h>

#include "graphics/D3D12GraphicEngine.h"
#include "window/GameWindowManager.h"
#include "framework/graphic/D3D12GraphicFrameWork.h"
#include "framework/gui/D3D12GuiFrameWork.h"
#include "framework/script/WindowsScriptWorkFrame.h"
#include "gui/D3D12GuiManager.h"
#include "core/logger/MyDebugLog.h"
#include "core/process/ProcessUtil.h"
#include "script/ScriptFunctionList.h"
#include "script/ScriptInstance.h"
#include "components/AllComponent.h"

#include <imgui/imgui.h>

/// /// @brief Windowsアプリケーションのテスト
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	// デバッグログの初期化
	QFE::MyDebugLog::GetInstance()->Initialize();

	// ゲームウィンドウマネージャの初期化とウィンドウの追加
	std::unique_ptr<QFE::GameWindowManager> gameWindowManager = std::make_unique<QFE::GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(1280, 720, "Test Window");
	HWND hWnd = gameWindowManager->GetWindow("Test Window");

	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine = 
		QFE::FRAMEWORK::CreateGraphicEngine(hWnd);
	// GUIマネージャの初期化
	std::unique_ptr<QFE::GUI::D3D12GuiManager> guiManager = 
		QFE::FRAMEWORK::CreateGuiManager(graphicEngine.get(), hWnd);

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

			guiManager->PostDraw();
			graphicEngine->PostDraw();
		}
	}

	guiManager->Shutdown();
	graphicEngine->Shutdown();
	gameWindowManager->Shutdown();
    return 0;
}
