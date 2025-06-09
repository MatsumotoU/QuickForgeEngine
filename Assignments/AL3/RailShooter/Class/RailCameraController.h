#pragma once
#include "Base/EngineCore.h"

class RailCameraController {
public:
	void Initialize(Camera* camera);
	void Update();

public:
	Camera* GetCamera() const;

private:
	Camera* camera_;
};