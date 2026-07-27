#include "DirectInputManager.h"
#include <cassert>
using namespace QFE;
using namespace QFE::INPUT;

void DirectInputManager::Initialize(const HWND& hwnd,const HINSTANCE& hInstance) {
	hInstance_ = hInstance;
	// directInput縺ｮ逕滓・
	directInput_ = nullptr;
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));
	hr;

	// Device縺ｮ逕滓・
	keyboard_.Initialize(hwnd,directInput_);
	mouse_.Initialize(hwnd,directInput_);
}

void DirectInputManager::Finalize() {
	
}

void DirectInputManager::Update() {
	// 繧ｭ繝ｼ繝懊・繝峨・蜃ｦ逅・
	keyboard_.Update();
	mouse_.Update();
}

MATH::Vector2 DirectInputManager::GetKeyMoveDir() {
	MATH::Vector2 result{};

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

	return result.Normalize();
}
