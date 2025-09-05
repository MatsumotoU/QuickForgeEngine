#pragma once
#include "Base/EngineCore.h"

class Player final {
public:
	Player() = default;
	~Player() = default;
	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update();
	void Draw();

	Vector2& GetMoveDir();
	Vector3 GetWorldPosition();
	Transform& GetTransform();
	bool& GetIsCanMove();
	bool& GetIsMoving();
	void SetMoveDir(const Vector2& dir);

private:
	EngineCore* engineCore_;
	Camera* camera_;
	std::unique_ptr<Model> model_;
	Transform transform_;
	float playerSize_;

	Vector3 velocity_;
	Vector3 acceleration_;
	float velocityDamping_;
	float accelerationDamping_;

	bool isCanMove_;
	bool isCanShot_;
	bool isMoving_;
	float moveTimer_;
	float maxMoveTimer_;
	float shotPower_;
	Vector2 moveDir_;
};