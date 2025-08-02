#pragma once
#include "Base/EngineCore.h"

class ShieldBar {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	void SetShieldPoint(float shieldPoint) { shieldPoint_ = shieldPoint; }
	float GetShieldPoint() const { return shieldPoint_; }

private:
	int frameCount_;
	Model model_;
	Model backModel_;
	EngineCore* engineCore_;
	float shieldPoint_;
	float maxShieldPoint_;
	float barWidth_;
	float barHeight_;
	Vector3 position_;
	Vector4 color_;
};