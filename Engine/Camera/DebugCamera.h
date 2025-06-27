#pragma once
#ifdef _DEBUG
#include "Camera.h"
#include "Model/Billboard.h"

class EngineCore;
class WinApp;
class DirectInputManager;

class DebugCamera {
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
	bool isDrawAnchor_;
	EngineCore* engineCore_;
	DirectInputManager* input_;
	Billboard anchorPointBillboard_;
	uint32_t anchorGH_;
};
#endif // _DEBUG
