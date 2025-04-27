#pragma once
#include <stdint.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class WinApp;

class DirectInputKeyboard {
public:
	DirectInputKeyboard();
	~DirectInputKeyboard();

public:// コアの関数
	void Initialize(WinApp* win, IDirectInput8* directInput);
	void Update();

public:// キー管理関数
	/// <summary>
	/// 任意のキーを押し続けているか
	/// </summary>
	/// <param name="DIK">DIK_Key</param>
	/// <returns></returns>
	bool GetPress(int8_t DIK);
	/// <summary>
	/// 任意のキーを押したか
	/// </summary>
	/// <param name="DIK">DIK_Key</param>
	/// <returns></returns>
	bool GetTrigger(int8_t DIK);
	/// <summary>
	/// 任意のキーを離したか
	/// </summary>
	/// <param name="DIK">DIK_Key</param>
	/// <returns></returns>
	bool GetRelease(int8_t DIK);

public:// キーボード操作変数
	BYTE key_[256];
	BYTE prekey_[256];

private:
	IDirectInputDevice8* CreateKeyboard();

private:
	IDirectInputDevice8* keyboard_;
	IDirectInput8* directInput_;
	WinApp* winApp_;
};
