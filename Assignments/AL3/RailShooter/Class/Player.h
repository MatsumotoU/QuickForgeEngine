#pragma once
#include "../../../../Engine/Base/EngineCore.h"

static inline const float kLimitMoveWidh = 7.0f;
static inline const float kLimitMoveHeight = 4.0f;

static inline const float kLimitSpeed = 8.0f;

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
	float moveSpeed_;

private:
	bool isActive_;

private:
	Model model_;
	EngineCore* engineCore_;
};