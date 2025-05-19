#include "DirectInputKeyboard.h"
#include "../../Base/Windows/WinApp.h"

#include <cassert>

DirectInputKeyboard::DirectInputKeyboard() {
	keyboard_ = nullptr;
	directInput_ = nullptr;
	winApp_ = nullptr;
}

DirectInputKeyboard::~DirectInputKeyboard() {
	keyboard_->Unacquire();
	keyboard_->Release();
}

void DirectInputKeyboard::Initialize(WinApp* win, IDirectInput8* directInput) {
	winApp_ = win;
	directInput_ = directInput;

	keyboard_ = CreateKeyboard();
}

void DirectInputKeyboard::Update() {
	// キーボードの処理
	memcpy(prekey_, key_, sizeof(key_));
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key_), key_);
}

bool DirectInputKeyboard::GetPress(uint32_t DIK) {
	if (key_[DIK]) {
		return true;
	}
	return false;
}

bool DirectInputKeyboard::GetTrigger(uint32_t DIK) {
	if (key_[DIK] && !prekey_[DIK]) {
		return true;
	}
	return false;
}

bool DirectInputKeyboard::GetRelease(uint32_t DIK) {
	if (!key_[DIK] && prekey_[DIK]) {
		return true;
	}
	return false;
}

IDirectInputDevice8* DirectInputKeyboard::CreateKeyboard() {
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
