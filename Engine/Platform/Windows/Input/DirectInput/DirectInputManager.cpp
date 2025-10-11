#include "DirectInputManager.h"
#include <cassert>

void DirectInputManager::Initialize(const HWND& hwnd,const HINSTANCE& hInstance) {
	hInstance_ = hInstance;
	// directInputの生成
	directInput_ = nullptr;
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));
	hr;

	// Deviceの生成
	keyboard_.Initialize(hwnd,directInput_);
	mouse_.Initialize(hwnd,directInput_);
}

void DirectInputManager::Update() {
	// キーボードの処理
	keyboard_.Update();
	mouse_.Update();
}

Vector2 DirectInputManager::GetKeyMoveDir() {
	Vector2 result{};

	if (keyboard_.GetPress(DIK_RIGHT) || keyboard_.GetPress(DIK_D)) {
		result.x += 1.0f;
	}
	if (keyboard_.GetPress(DIK_LEFT) || keyboard_.GetPress(DIK_A)) {
		result.x -= 1.0f;
	}
	if (keyboard_.GetPress(DIK_UP) || keyboard_.GetPress(DIK_W)) {
		result.y += 1.0f;
	}
	if (keyboard_.GetPress(DIK_DOWN) || keyboard_.GetPress(DIK_S)) {
		result.y -= 1.0f;
	}

	/*if (!keyboard_.GetPress(DIK_RIGHT) && !keyboard_.GetPress(DIK_LEFT)) {
		result.x += 0.0f;
	}
	if (!keyboard_.GetPress(DIK_UP) && !keyboard_.GetPress(DIK_DOWN)) {
		result.y += 0.0f;
	}*/

	return result.Normalize();
}
