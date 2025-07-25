#pragma once
#include "../../../../Engine/Base/EngineCore.h"
class Block {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	Transform transform_;
	
private:
	Model model_;
	EngineCore* engineCore_;
	
};