#include "WindowsEngineCore.h"

#ifdef _DEBUG
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

WindowsEngineCore::WindowsEngineCore(HINSTANCE& hInstance, LPSTR& lpCmdLine) :hInstance_(hInstance),lpCmdLine_(lpCmdLine){
}

void WindowsEngineCore::Initialize() {
	gameWindowManager = std::make_unique<GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow(1280, 720, "QuickForgeEngine");
}

void WindowsEngineCore::MainLoop() {
	while (gameWindowManager->IsWindowActive())
	{
		// メッセージキューからメッセージを取得
		MSG msg;
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break; // アプリケーション終了
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);

		} else {
			gameWindowManager->Update();
			gameWindowManager->Draw();
		}
	}
}

void WindowsEngineCore::Shutdown() {
	gameWindowManager->Shutdown();
#ifdef _DEBUG
	DebugLog("FinalizeEngine");
#endif // _DEBUG
}
