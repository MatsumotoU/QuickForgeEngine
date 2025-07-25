#pragma once
#include "../../../../Engine/Base/EngineCore.h"
#include "BaseEnemyState.h"
#include "../../../../Engine/Utility/TimeCall.h"

static inline const float kXLimit = 20.0f;
static inline const uint32_t kMaxShotInterval = 30;

enum class Phase {
	Approach,
	Leave,
};

enum class MoveType {
	Normal,
	Sin,
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
	void Shot();

public:
	void Spawn(Vector3 position, Vector3 velocity,uint32_t moveType);

public:
	void SetIsShot(bool isShot);

public:
	bool GetIsActive();
	Phase GetPhase();
	bool GetIsShot();
	Matrix4x4 GetRotateMatrix();
	Vector3 GetDir();
	Vector3 GetWorldPosition();

public:
	MoveType moveType_;
	Transform transform_;

private:
	//static void (Enemy::*spFuncTable[])();
	std::unique_ptr<BaseEnemyState> state_;

private:
	float frameCount_;
	bool isActive_;
	Vector3 velocity_;
	Phase phase_;
	float leaveSpeed_;
	
	bool isShot_;
	uint32_t shotInterval_;

private:
	EngineCore* engineCore_;
	Model model_;

	std::list<TimeCall*> timedCalls_;

};