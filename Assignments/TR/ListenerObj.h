#pragma once
#include "../../Engine/Base/EngineCore.h"

class ListenerObj {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

public:
	Transform transform_;

private:
	EngineCore* engineCore_;
	Model model_;

};