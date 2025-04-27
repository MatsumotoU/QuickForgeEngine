#pragma once
#include <Windows.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

// デバイス単位でクラス分け
class WinApp;

class DirectInputManager {
public:
	void Initialize(WinApp* winApp, const HINSTANCE& hInstance);
	void Update();

private:// デバイス作成系
	IDirectInputDevice8* CreateKeyboardDivice();

private:// DirectInputのコア
	HINSTANCE hInstance_;
	WinApp* winApp_;
	IDirectInput8* directInput_;

private:// 入力デバイス
	IDirectInputDevice8* keybord_;

public:// キーボード操作変数
	BYTE key_[256];
	BYTE prekey_[256];
};