#include "QuickForgeEngine.h"

#include "Window/GameWindowManager.h"

#ifdef _DEBUG
#include "Utility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void QuickForgeEngine::Run(HINSTANCE& hInstance, LPSTR& lpCmdLine) {
	hInstance; lpCmdLine;
	// エンジンの初期化
#ifdef _DEBUG
	DebugLog("StartEngine");
#endif // _DEBUG
	std::unique_ptr<IGameWindowManager> gameWindowManager = std::make_unique<GameWindowManager>();
	gameWindowManager->Initialize();
	gameWindowManager->AddWindow( 1280, 720, "QuickForgeEngine");

	// メインループ
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
	gameWindowManager->Shutdown();

	// 終了処理
#ifdef _DEBUG
	DebugLog("FinalizeEngine");
#endif // _DEBUG
}
