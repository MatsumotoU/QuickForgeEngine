#pragma once
#include "../../../../Engine/Base/EngineCore.h"
#include "BaseEnemyState.h"
#include "../../../../Engine/Utility/TimeCall.h"
#include "Colliders/Collider.h"
#include <functional>
#include <map>

static inline const float kXLimit = 20.0f;
static inline const uint32_t kMaxShotInterval = 120;

enum class Phase {
	Approach,
	Leave,
};

enum class MoveType {
	Normal,
	Sin,
};

class Enemy : public Collider{
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	void OnCollision(const nlohmann::json& otherData) override;
	void HitRevenge(int level);

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
	void SetIsActive(bool isActive);

public:
	bool GetIsShield();
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
	void NormalMotion();
	void DamageMotion();
	void DeadMotion();

private:
	enum MotionState {
		NORMAL,
		DAMAGE,
		DEAD,
	};
	std::map<MotionState, std::function<void()>> motionFuncMap_;
	MotionState motionState_;
	float motionTime_;
	std::unique_ptr<BaseEnemyState> state_;

private:
	float frameCount_;
	bool isActive_;
	Vector3 velocity_;
	Phase phase_;
	float leaveSpeed_;
	int hitPoint_;
	int maxHitPoint_;
	
	bool isShield_;
	bool isShot_;
	uint32_t shotInterval_;

private:
	EngineCore* engineCore_;
	Model model_;
	Model shieldModel_;

	//std::list<TimeCall*> timedCalls_;

};