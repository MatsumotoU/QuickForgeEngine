#include "DirectInputMouse.h"
#include "../../Base/Windows/WinApp.h"

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include <cassert>

DirectInputMouse::DirectInputMouse() {
	mouse_ = nullptr;
	directInput_ = nullptr;
	winApp_ = nullptr;
	mouseState_ = {};
	preMouseState_ = {};
	mousePos_ = {};
	wheelDir_ = 0.0f;

	mouseScreenPos_ = {};
	preMouseScreenPos_ = {};
}

DirectInputMouse::~DirectInputMouse() {
	mouse_->Unacquire();
	mouse_->Release();
}

void DirectInputMouse::Initialize(WinApp* win, IDirectInput8* directInput) {
	winApp_ = win;
	directInput_ = directInput;

	mouse_ = CreateMouse();
}

void DirectInputMouse::Update() {
	preMouseState_ = mouseState_;
	preMouseScreenPos_ = mouseScreenPos_;
	mouse_->Acquire();
	mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);

	// スクリーン上の座標に変換
	GetCursorPos(&mousePos_);
	ScreenToClient(winApp_->GetHWND(),&mousePos_);
	mouseScreenPos_.x = static_cast<float>(mousePos_.x);
	mouseScreenPos_.y = static_cast<float>(mousePos_.y);

	mouseMoveDir_.x = static_cast<float>(mouseState_.lX);
	mouseMoveDir_.y = static_cast<float>(mouseState_.lY);
	wheelDir_ = static_cast<float>(mouseState_.lZ);
	mouseMoveDir_ = mouseMoveDir_.Normalize();

	// 移動量計算
	deltaMouse_ = mouseScreenPos_ - preMouseScreenPos_;
}

bool DirectInputMouse::GetPress(int8_t DIK) {
	if (mouseState_.rgbButtons[DIK]) {
		return true;
	}
	return false;
}

bool DirectInputMouse::GetTrigger(int8_t DIK) {
	if (mouseState_.rgbButtons[DIK] && !preMouseState_.rgbButtons[DIK]) {
		return true;
	}
	return false;
}

bool DirectInputMouse::GetRelease(int8_t DIK) {
	if (!mouseState_.rgbButtons[DIK] && preMouseState_.rgbButtons[DIK]) {
		return true;
	}
	return false;
}

IDirectInputDevice8* DirectInputMouse::CreateMouse() {
	IDirectInputDevice8* mouse = nullptr;
	HRESULT hr = directInput_->CreateDevice(GUID_SysMouse, &mouse, NULL);
	assert(SUCCEEDED(hr));

	hr = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));

	hr = mouse->SetCooperativeLevel(
		winApp_->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

	return mouse;
}
