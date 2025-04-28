#pragma once
#ifdef _DEBUG
#include "Camera.h"
class WinApp;
class DirectInputManager;

class DebugCamera {
public:
	void Initialize(WinApp* win, DirectInputManager* input);
	void Update();
	void DrawImGui();

public:
	Camera camera_;
	Transform transform_;
	float mouseSensitivity_;

private:
	DirectInputManager* input_;
	
};
#endif // _DEBUG
