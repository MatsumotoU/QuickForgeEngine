#pragma once
#include "../../../../Engine/Base/EngineCore.h"

class Bullet {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

public:
	void ShotBullet(Vector3 position, Vector3 velocity, uint32_t aliveTime);

public:
	bool GetIsActive();

private:
	bool isActive_;
	uint32_t aliveTime_;
	uint32_t maxAliveTime_;
	Vector3 velocity_;
	Transform transform_;

private:
	EngineCore* engineCore_;
	Model model_;

};