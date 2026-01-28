/**
 * @file DirectInputMouse.cpp
 * @brief DirectInputを使用したマウス入力管理クラスの実装
 */
#include "engine/include/input/DirectInput/DirectInputMouse.h"
#include <cassert>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

namespace QFE {

	DirectInputMouse::DirectInputMouse() {
		mouse_ = nullptr;
		directInput_ = nullptr;
		mouseState_ = {};
		preMouseState_ = {};
		mousePos_ = {};
		wheelDir_ = 0;

		mouseScreenPos_ = {};
		preMouseScreenPos_ = {};
	}

	DirectInputMouse::~DirectInputMouse() {
		if (mouse_) {
			mouse_->Unacquire();
			mouse_->Release();
		}
	}

	void DirectInputMouse::Initialize(const HWND& hwnd, IDirectInput8* directInput) {
		directInput_ = directInput;
		hwnd_ = hwnd;
		mouse_ = CreateMouse();
	}

	void DirectInputMouse::Update() {
		preMouseState_ = mouseState_;
		preMouseScreenPos_ = mouseScreenPos_;
		mouse_->Acquire();
		mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);

		// マウスポジション取得
		GetCursorPos(&mousePos_);
		ScreenToClient(hwnd_, &mousePos_);
		mouseScreenPos_.x = static_cast<float>(mousePos_.x);
		mouseScreenPos_.y = static_cast<float>(mousePos_.y);

		mouseMoveDir_.x = static_cast<float>(mouseState_.lX);
		mouseMoveDir_.y = static_cast<float>(mouseState_.lY);
		wheelDir_ = static_cast<float>(mouseState_.lZ);
		mouseMoveDir_ = mouseMoveDir_.Normalize();


		deltaMouse_ = mouseScreenPos_ - preMouseScreenPos_;
	}

	bool DirectInputMouse::GetPress(int8_t DIK) {
		if (mouseState_.rgbButtons[DIK] & 0x80) {
			return true;
		}
		return false;
	}

	bool DirectInputMouse::GetTrigger(int8_t DIK) {
		if (mouseState_.rgbButtons[DIK] & 0x80 && !(preMouseState_.rgbButtons[DIK] & 0x80)) {
			return true;
		}
		return false;
	}

	bool DirectInputMouse::GetRelease(int8_t DIK) {
		if (!(mouseState_.rgbButtons[DIK] & 0x80) && preMouseState_.rgbButtons[DIK] & 0x80) {
			return true;
		}
		return false;
	}

	IDirectInputDevice8* DirectInputMouse::CreateMouse() {
		IDirectInputDevice8* mouseDevice;
		HRESULT hr = directInput_->CreateDevice(GUID_SysMouse, &mouseDevice, NULL);
		assert(SUCCEEDED(hr));

		hr = mouseDevice->SetDataFormat(&c_dfDIMouse);
		assert(SUCCEEDED(hr));

		hr = mouseDevice->SetCooperativeLevel(hwnd_, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
		assert(SUCCEEDED(hr));

		return mouseDevice;
	}

}
