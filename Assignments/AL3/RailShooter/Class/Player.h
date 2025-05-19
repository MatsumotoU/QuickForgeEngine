#pragma once
#include "../../../../Engine/Base/EngineCore.h"

class Player {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	Transform transform_;

public:
	bool GetIsActive();

public:
	void SetIsActive(bool isActive);

public:
	Vector3 velocity_;
	Vector3 acceleration_;

private:
	bool isActive_;

private:
	Model model_;
	EngineCore* engineCore_;
};