#include "engine/include/input/DirectInput/DirectInputKeyboard.h"

#include <cassert>
using namespace QFE;
DirectInputKeyboard::DirectInputKeyboard(): 
	keyboard_(nullptr), directInput_(nullptr), pressedKeys_(256) {
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
	memcpy(preKey_, key_, sizeof(key_));
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key_), key_);

	pressedKeys_.clear();
}

bool DirectInputKeyboard::GetPress(uint32_t DIK) {
	if (key_[DIK]) {
		return true;
	}
	return false;
}

bool DirectInputKeyboard::GetTrigger(uint32_t DIK) {
	if (key_[DIK] && !preKey_[DIK]) {
		return true;
	}
	return false;
}

bool DirectInputKeyboard::GetRelease(uint32_t DIK) {
	if (!key_[DIK] && preKey_[DIK]) {
		return true;
	}
	return false;
}

const SafeVector<uint32_t>& DirectInputKeyboard::GetPressedKeys()
{
	for (uint32_t keyCode = 0; keyCode < 256; keyCode++) {
		if (key_[keyCode]) {
			pressedKeys_.push_back(keyCode);
		}
	}
	return pressedKeys_;
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
