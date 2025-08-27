#pragma once
#include "Base/EngineCore.h"

class Ice final{
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

	Transform transform_;
	Vector4 syropeColor_;

private:
	EngineCore* engineCore_;
	Model iceModel_;
	Model syropeModel_;
};