#pragma once
#include "../../../../Engine/Base/EngineCore.h"

class Enemy {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	Transform transform_;

public:
	float frameCount_;
	Vector3 velocity_;
	Vector3 acceleration_;

private:
	Model model_;
	EngineCore* engineCore_;

};