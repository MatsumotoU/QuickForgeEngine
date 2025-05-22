#pragma once
#include "../../../../Engine/Base/EngineCore.h"
#include "BaseEnemyState.h"

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
	void ChangeState(std::unique_ptr<BaseEnemyState> state);

public:
	void Approch();
	void Leave();

public:
	void Spawn(Vector3 position, Vector3 velocity);

public:
	bool GetIsActive();
	Phase GetPhase();

private:
	//static void (Enemy::*spFuncTable[])();
	std::unique_ptr<BaseEnemyState> state_;

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