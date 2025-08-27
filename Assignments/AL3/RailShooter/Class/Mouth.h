#pragma once
#include "Base/EngineCore.h"

class Mouth {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	float openRadian_;
	Transform transform_;

private:
	EngineCore* engineCore_;
	Model topMouthModel_;
	Model underMouthModel_;

	Transform topMouthTransform_;
	Transform underMouthTransform_;
};