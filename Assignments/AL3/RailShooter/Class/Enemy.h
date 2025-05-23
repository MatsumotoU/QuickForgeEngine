#pragma once
#include "../../../../Engine/Base/EngineCore.h"
#include "BaseEnemyState.h"

static inline const float kXLimit = 20.0f;
static inline const uint32_t kMaxShotInterval = 30;

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
	void ChangeState(std::unique_ptr<BaseEnemyState> state);

public:
	void Approch();
	void Leave();

public:
	void Spawn(Vector3 position, Vector3 velocity);

public:
	void SetIsShot(bool isShot);

public:
	bool GetIsActive();
	Phase GetPhase();
	bool GetIsShot();
	Matrix4x4 GetRotateMatrix();

public:
	Transform transform_;

private:
	//static void (Enemy::*spFuncTable[])();
	std::unique_ptr<BaseEnemyState> state_;

private:
	bool isActive_;
	Vector3 velocity_;
	Phase phase_;
	float leaveSpeed_;
	
	bool isShot_;
	uint32_t shotInterval_;

private:
	EngineCore* engineCore_;
	Model model_;

};