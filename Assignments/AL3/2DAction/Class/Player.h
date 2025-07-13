#pragma once
#include "../../../../Engine/Base/EngineCore.h"

static inline const float kAcceleration = 2.0f;
static inline const float kMaxAcceleration = 10.0f;
static inline const float kMaxVelocity = 10.0f;
static inline const float kJumpPower = 30.0f;

static inline const float kTimeTurn = 0.5f; // 回転にかかる時間
static inline const float kWith = 1.0f;

class MapChip;

enum class LRDirection {
	kLeft,
	kRight,
};

enum class BahaviorType {
	kIdle,
	kMove,
	kAttack,
};

class Player {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	Transform transform_;

public:
	void Move();
	void Attack();

public:
	bool GetIsActive();

public:
	void SetIsActive(bool isActive);
	void SetMap(MapChip* map);

public:
	Vector3 velocity_;
	Vector3 acceleration_;

private:
	bool isActive_;

private:
	bool isGround_;
	bool isLanding_;

private:
	float nowTurnY_;
	float turnTable[2];
	float turnTime_;
	LRDirection lrDirection_;

private:
	BahaviorType bahaviorType_;
	bool isAttacking_;
	float attackChargeTime_;
	Billboard billboard_;

private:
	MapChip* map_;

private:
	Model model_;
	EngineCore* engineCore_;
};