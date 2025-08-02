#pragma once
#include "../../../../Engine/Base/EngineCore.h"
#include "../../../../Engine/Colliders/Collider.h"
#include <map>
#include <functional>

static inline const float kLimitMoveWidh = 2.3f;
static inline const float kLimitMoveHeight = 1.5f;

static inline const float kLimitSpeed = 8.0f;

class Player : public Collider{
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	Transform transform_;

public:
	void OnCollision(const nlohmann::json& otherData) override;

public:
	bool GetIsActive();
	bool GetIsShot();
	Matrix4x4 GetParentMatrix();
	Matrix4x4 GetParentWorldMatrix();
	Matrix4x4 GetWorldMatrix();
	Matrix4x4 GetRotateMatrix();
	Vector3 GetWorldPosition() override;
	Vector3 GetScreenPosition(Camera* camera);
	Vector3 GetDir();

public:
	void SetParent(const Matrix4x4& parentMatrix);
	void SetIsActive(bool isActive);
	void SetIsShot(bool isShot);

private:
	void NormalMotion();
	void DamageMotion();

public:
	int frameCount_;
	Vector3 velocity_;
	Vector3 acceleration_;
	float moveSpeed_;

	enum MotionState
	{
		NORMAL,
		DAMAGE,
		SHIELD,
		BREAKING,
	};

	MotionState motionState_;

private:
	Matrix4x4 parentMatrix_;
	std::map<MotionState, std::function<void()>> motionFuncMap_;
	float motionTime_;

private:
	bool isActive_;
	bool isShot_;
	bool isShield_;
	bool isBreaking_;

private:
	float shotCooldown_;
	float maxShotCooldown_;
	float sheildPoint_;
	int hitPoint_;

private:
	Model model_;
	Model shieldModel_;
	EngineCore* engineCore_;
	BlendMode blendMode_;
	int blendNum_;
};