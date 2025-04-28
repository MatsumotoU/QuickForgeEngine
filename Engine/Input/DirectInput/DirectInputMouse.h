#pragma once
#include <stdint.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include "../../Math/Vector/Vector2.h"
#include "../../Math/Vector/Vector3.h"

class WinApp;

class DirectInputMouse {
public:
	DirectInputMouse();
	~DirectInputMouse();

public:// コアの関数
	void Initialize(WinApp* win, IDirectInput8* directInput);
	void Update();

public:// マウス管理関数
	/// <summary>
	/// 任意のキーを押し続けているか
	/// </summary>
	/// <param name="DIK">左クリック[0],右クリック[1],中クリック[2]</param>
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

public:// マウス操作変数
	DIMOUSESTATE mouseState_;
	DIMOUSESTATE preMouseState_;
	Vector2 mouseScreenPos_;
	Vector2 mouseMoveDir_;
	float wheelDir_;

private:
	IDirectInputDevice8* CreateMouse();

private:
	IDirectInputDevice8* mouse_;
	IDirectInput8* directInput_;
	WinApp* winApp_;
	POINT mousePos_;
};
