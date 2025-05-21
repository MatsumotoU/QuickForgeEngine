#pragma once
#include "../../../../Engine/Base/EngineCore.h"

static inline const float kXLimit = 20.0f;

enum class Phase {
	Approach,
	Leave,
};

class Enemy {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

public:
	void Spawn(Vector3 position, Vector3 velocity);

public:
	bool GetIsActive();

private:
	bool isActive_;
	Vector3 velocity_;
	Transform transform_;
	Phase phase_;
	float leaveSpeed_;

private:
	EngineCore* engineCore_;
	Model model_;

};