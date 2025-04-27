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
	keyboard_.Initialize(winApp, directInput_);
	mouse_.Initialize(winApp, directInput_);
}

void DirectInputManager::Update() {
	// キーボードの処理
	keyboard_.Update();
	mouse_.Update();
}