#pragma once
#include "../../../../Engine/Base/EngineCore.h"
class SkyDome {
public:
	void Initialize(EngineCore* engineCore);
	void Draw(Camera* camera);

private:
	EngineCore* engineCore_;
	Model model_;
	Transform transform_;
};