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
	hr;

	// Deviceの生成
	keyboard_.Initialize(winApp, directInput_);
	mouse_.Initialize(winApp, directInput_);
}

void DirectInputManager::Update() {
	// キーボードの処理
	keyboard_.Update();
	mouse_.Update();
}

Vector2 DirectInputManager::GetKeyMoveDir() {
	Vector2 result{};

	if (keyboard_.GetPress(DIK_RIGHT)) {
		result.x += 1.0f;
	}
	if (keyboard_.GetPress(DIK_LEFT)) {
		result.x -= 1.0f;
	}
	if (keyboard_.GetPress(DIK_UP)) {
		result.y += 1.0f;
	}
	if (keyboard_.GetPress(DIK_DOWN)) {
		result.y -= 1.0f;
	}

	if (!keyboard_.GetPress(DIK_RIGHT) && !keyboard_.GetPress(DIK_LEFT)) {
		result.x += 0.0f;
	}
	if (!keyboard_.GetPress(DIK_UP) && !keyboard_.GetPress(DIK_DOWN)) {
		result.y += 0.0f;
	}

	return result.Normalize();
}
