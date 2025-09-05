// Debug
#ifdef _DEBUG
#include "Engine/Base/Windows/WinAppDebugCore.h"
#include "Engine/Base/DirectX/DirectX12DebugCore.h"
#endif // _DEBUG

// 現状ないと動かないコアたち
#include "Engine/Base/EngineCore.h"
#include "LegacyScene/SceneManager.h"

// windowsアプリでのエントリーポイント(main関数) 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int) {
	// * ゲーム以前の設定 * //
#ifdef _DEBUG
	WinAppDebugCore winAppDbgCore(lpCmdLine);
	DirectX12DebugCore directXDbgCore;
#endif // _DEBUG

	// エンジンの初期化
	MSG msg{};
	EngineCore engineCore;
	engineCore.Initialize(L"2001_モグラ相撲", hInstance, lpCmdLine,&msg);

	SceneManager sceneManager(&engineCore);
	sceneManager.Initialize();

	// ウィンドウのXボタンが押されるまでループ
	while (engineCore.GetWinApp()->GetIsWindowQuit()) {
		// windowにメッセージが基底たら最優先で処理される
		if (engineCore.GetWinApp()->GetCanLoop()) {
			// === Update ===
			engineCore.Update();
			sceneManager.Update();

			// === Draw ===
			engineCore.PreDraw();
			sceneManager.Draw();

			// === EndDraw ===
			engineCore.PostDraw();
		}
	}
	return 0;
}