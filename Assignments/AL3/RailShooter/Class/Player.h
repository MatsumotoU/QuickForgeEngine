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
	void onCollision();

public:
	bool GetIsActive();
	bool GetIsShot();
	Matrix4x4 GetWorldMatrix();
	Matrix4x4 GetRotateMatrix();

public:
	void SetIsActive(bool isActive);
	void SetIsShot(bool isShot);

public:
	Vector3 velocity_;
	Vector3 acceleration_;
	float moveSpeed_;

private:
	bool isActive_;
	bool isShot_;

private:
	float shotCooldown_;
	float maxShotCooldown_;

private:
	Model model_;
	EngineCore* engineCore_;
	BlendMode blendMode_;
	int blendNum_;
};