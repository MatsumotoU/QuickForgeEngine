#pragma once
#include "Base/EngineCore.h"

class KeyTutorial {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	bool isActive_;
	Transform transform_;

private:
	EngineCore* engineCore_;
	Model keyTutorialModel_;
	Model buttonTutorialModel_;
};