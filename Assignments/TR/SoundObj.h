#pragma once
#include "../../Engine/Base/EngineCore.h"

class SoundObj {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

	void SetTarget(Vector3* target);

public:
	Transform transform_;
	Vector3* lookTargetPosition_;

private:
	EngineCore* engineCore_;
	Model model_;
};