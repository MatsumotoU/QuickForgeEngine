#pragma once
#include "Base/EngineCore.h"

class HitEffect {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

	void EmmitEffect(const Vector3& pos);

public:
	bool isActive_;
	Transform transform_;

private:
	float effectDuration_; // エフェクトの持続時間
	float effectElapsedTime_; // 経過時間

	EngineCore* engineCore_;
	Billboard billcoad[3];
	uint32_t textureHandle_;
};