#include "engine/include/input/DirectInput/DirectInputKeyboard.h"

#include <cassert>

DirectInputKeyboard::DirectInputKeyboard() {
	keyboard_ = nullptr;
	directInput_ = nullptr;
}

DirectInputKeyboard::~DirectInputKeyboard() {
	keyboard_->Unacquire();
	keyboard_->Release();
}

void DirectInputKeyboard::Initialize(const HWND& hwnd, IDirectInput8* directInput) {
	directInput_ = directInput;
	hwnd_ = hwnd;
	keyboard_ = CreateKeyboard();
}

void DirectInputKeyboard::Update() {
	// 繧ｭ繝ｼ繝懊・繝峨・蜃ｦ逅・
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
		hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	return keyboard;
}
