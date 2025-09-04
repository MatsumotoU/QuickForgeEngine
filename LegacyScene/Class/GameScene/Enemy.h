#pragma once
#include "Base/EngineCore.h"

class Enemy final {
public:
	Enemy() = default;
	~Enemy() = default;
	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();
	Vector3 GetWorldPosition();
	Transform& GetTransform();

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
	float playerSize_;

	Vector3 velocity_;
	Vector3 acceleration_;
	float velocityDamping_;
	float accelerationDamping_;

	bool isCanMove_;
	bool isCanShot_;
	bool isMoving_;
	bool isReqestBuilding_;
	bool isJumping_;
	bool isGrounded_;
	float moveTimer_;
	float maxMoveTimer_;
	float shotPower_;
	Vector2 moveDir_;
};