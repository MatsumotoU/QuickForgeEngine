#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp final {
public:
	// ウィンドウサイズ
	static const int kWindowWidth = 1280; // 横幅
	static const int kWindowHeight = 720; // 縦幅
	// ウィンドウクラス名
	static const wchar_t kWindowClassName[];

public:
	/// <summary>
	/// ウィンドウプロシージャ
	/// </summary>
	/// <param name="hwnd">ウィンドウハンドル</param>
	/// <param name="msg">メッセージ番号</param>
	/// <param name="wparam">メッセージ情報1</param>
	/// <param name="lparam">メッセージ情報2</param>
	/// <returns>成否</returns>
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

public:
	/// <summary>
	/// ゲームウィンドウの作成
	/// <param name="title">ウィンドウタイトル</param>
	/// <param name="windowStyle">ウィンドウの初期スタイル</param>
	/// <param name="clientWidth">ウィンドウのクライアント領域の初期幅</param>
	/// <param name="clientHeight">ウィンドウのクライアント領域の初期高さ</param>
	/// </summary>
	void CreateGameWindow(const LPCWSTR& windowName, int32_t clientWidth = kWindowWidth, int32_t clientHeight = kWindowHeight);

public:
	void SetMSG(MSG* msg);

public:
	bool GetCanLoop();
	bool GetIsWindowQuit();
	HWND GetHWND();

private:
	WNDCLASS wc{};
	HWND hwnd;
	MSG* msg_;
};

