#pragma once
#include "Base/EngineCore.h"

class Number {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	
	Model* GetCerrentModel();

	Transform transform_;
	int value_;

private:
	std::array<Model,10> models_;
	EngineCore* engineCore_ = nullptr;
};