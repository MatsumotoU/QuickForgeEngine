#pragma once
#include "DirectInputKeyboard.h"
#include "DirectInputMouse.h"

#include <Windows.h>

class DirectInputManager final {
public:
	DirectInputManager() = default;
	~DirectInputManager() = default;

	void Initialize(const HWND& hwnd,const HINSTANCE& hInstance);
	void Finalize();
	void Update();

public:
	/// <summary>
	/// ゲームでありがちな移動操作の方向を返します
	/// </summary>
	/// <returns></returns>
	Vector2 GetKeyMoveDir();

private:// DirectInputのコア
	HINSTANCE hInstance_;
	IDirectInput8* directInput_;

public:
	DirectInputKeyboard keyboard_;
	DirectInputMouse mouse_;
};
