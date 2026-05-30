#include "WindowGenerater.h"

namespace QFE {

	void WindowGenerator::CreateGameWindow(WNDCLASS& wc, HWND& hwnd, WindowConfigData& config, WNDPROC& proc, WindowEventsManager* eventManager) {
		// ウィンドウプロージャ
		wc.lpfnWndProc = proc;
		// ウィンドウクラス名
		wc.lpszClassName = L"CG2WindowClass";
		// インスタンスハンドル
		wc.hInstance = GetModuleHandle(nullptr);
		// カーソル
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

		// ウィンドウクラスを登録する
		RegisterClass(&wc);

		// ウィンドウサイズを表す構造体にクライアント領域を入れる
		RECT wrc = { 0,0,config.clientWidth,config.clientHeight };

		// クライアント領域を元に実際のサイズにwrcを更新してもらう
		AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

		// リサイズ抑止
		DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		
		// ウィンドウの生成
		hwnd = CreateWindow(
			wc.lpszClassName,
			config.windowName.c_str(),
			style,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			wrc.right - wrc.left,
			wrc.bottom - wrc.top,
			nullptr,
			nullptr,
			wc.hInstance,
			eventManager
		);

		ShowWindow(hwnd, SW_SHOW);

		// ウィンドウの機能解放
		if (config.isCanDropFiles) {
			// ウィンドウに対して、ドラッグ・ドロップを有効にする
			DragAcceptFiles(hwnd, TRUE);
		}

	}

}
