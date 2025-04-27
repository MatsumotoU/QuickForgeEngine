#include "DirectInputManager.h"
#include "../../Base/Windows/WinApp.h"
#include <cassert>

void DirectInputManager::Initialize(WinApp* winApp, const HINSTANCE& hInstance) {
	winApp_ = winApp;
	hInstance_ = hInstance;
	// directInputの生成
	directInput_ = nullptr;
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));

	// Deviceの生成
	keybord_ = CreateKeyboardDivice();
}

void DirectInputManager::Update() {
	// キーボードの処理
	memcpy(prekey_, key_, sizeof(prekey_));
	keybord_->Acquire();
	keybord_->GetDeviceState(sizeof(key_), key_);
}

IDirectInputDevice8* DirectInputManager::CreateKeyboardDivice() {
	IDirectInputDevice8* keyboard = nullptr;
	HRESULT hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));

	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	hr = keyboard->SetCooperativeLevel(
		winApp_->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	return keyboard;
}
