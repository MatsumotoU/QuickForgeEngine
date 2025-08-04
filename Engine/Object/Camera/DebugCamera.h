#pragma once
#ifdef _DEBUG
#include "Camera.h"
#include <functional>

class EngineCore;
class WinApp;
class DirectInputManager;

class DebugCamera {
public:
	DebugCamera() = delete;
	DebugCamera(EngineCore* engineCore);
	~DebugCamera();

public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void DrawImGui();

public:
	Camera camera_;
	Transform transform_;
	float mouseSensitivity_;
	Vector3 anchorPoint_;

private:
	EngineCore* engineCore_;
	DirectInputManager* input_;
};
#endif // _DEBUG
