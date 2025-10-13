#pragma once
#include "Utility/DesignPatterns/Singleton.h"

#include "DirectInputKeyboard.h"
#include "DirectInputMouse.h"

#include <Windows.h>

class DirectInputManager : public Singleton<DirectInputManager>{
	friend class Singleton<DirectInputManager>;
	DirectInputManager() = default;
	DirectInputManager(const DirectInputManager&) = delete;
	DirectInputManager& operator=(const DirectInputManager&) = delete;
	DirectInputManager(DirectInputManager&&) = delete;
	DirectInputManager& operator=(DirectInputManager&&) = delete;

public:
	void Initialize(const HWND& hwnd,const HINSTANCE& hInstance);
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