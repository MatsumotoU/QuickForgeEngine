#pragma once
#include "Base/EngineCore.h"

class PlayerHitPoint {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

	void SetHitPoint(int p);

private:
	Model model_[3];
	EngineCore* engineCore_;
	Vector3 pos_;
	int hitp_;
};