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

private:// DirectInputのコア
	HINSTANCE hInstance_;
	WinApp* winApp_;
	IDirectInput8* directInput_;

public:
	DirectInputKeyboard keyboard_;
	DirectInputMouse mouse_;
};