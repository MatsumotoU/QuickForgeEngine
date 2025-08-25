#pragma once
#include "Base/EngineCore.h"

class Ground final{
public:
	void Init(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
private:
	Model groundModel_;
	EngineCore* engineCore_;
};