#include "WinApp.h"

#ifdef _DEBUG
#include "../../../externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

LRESULT WinApp::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

#ifdef _DEBUG
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif // _DEBUG


	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
	// ウィンドウが破棄された
	case WM_DESTROY:
		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);

}

void WinApp::CreateGameWindow(const LPCWSTR& windowName,int32_t clientWidth, int32_t clientHeight) {

	// ウィンドウプロージャー
	wc.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	// インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc);

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,clientWidth,clientHeight };

	// クライアント領域を元に実際のサイズにwrcを更新してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	hwnd = CreateWindow(
		wc.lpszClassName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr
	);

	ShowWindow(hwnd, SW_SHOW);
}

void WinApp::SetMSG(MSG* msg) {
	msg_ = msg;
}

bool WinApp::GetCanLoop() {
	if (PeekMessage(msg_, NULL, 0, 0, PM_REMOVE)) {
		TranslateMessage(msg_);
		DispatchMessage(msg_);
		return false;
	}
	return true;
}

bool WinApp::GetIsWindowQuit() {
	if (msg_->message != WM_QUIT) {
		return true;
	}
	return false;
}

HWND WinApp::GetHWND() {
	return hwnd;
}
