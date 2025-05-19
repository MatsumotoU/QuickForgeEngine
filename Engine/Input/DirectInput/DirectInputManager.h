#pragma once
#include "DirectInputKeyboard.h"
#include "DirectInputMouse.h"

#include <Windows.h>

// デバイス単位でクラス分け
class WinApp;

class DirectInputManager {
public:
	void Initialize(WinApp* winApp, const HINSTANCE& hInstance);
	void Update();

public:
	/// <summary>
	/// ゲームでありがちな移動操作の方向を返します
	/// </summary>
	/// <returns></returns>
	Vector2 GetKeyMoveDir();

private:// DirectInputのコア
	HINSTANCE hInstance_;
	WinApp* winApp_;
	IDirectInput8* directInput_;

public:
	DirectInputKeyboard keyboard_;
	DirectInputMouse mouse_;
};