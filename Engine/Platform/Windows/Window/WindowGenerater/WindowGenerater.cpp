#include "WindowGenerater.h"

void WindowGenerater::CreateGameWindow(WNDCLASS& wc, HWND& hwnd, WindowConfigData& config, WNDPROC& proc, WindowEventsManager* eventManager) {
	// ウィンドウプロージャー
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

	// ウィンドウの生成
	hwnd = CreateWindow(
		wc.lpszClassName,
		config.windowName.c_str(),
		WS_OVERLAPPEDWINDOW,
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
		// ウィンドウに対して、ドラッグ＆ドロップを有効にする
		DragAcceptFiles(hwnd, TRUE);
	}

}
