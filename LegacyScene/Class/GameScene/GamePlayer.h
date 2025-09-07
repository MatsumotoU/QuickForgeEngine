#pragma once
#include "Base/EngineCore.h"

class GamePlayer {
public:
	GamePlayer() = default;
	~GamePlayer() = default;
	virtual void Initialize(EngineCore* engineCore, Camera* camera) = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	void ResetForce();
	void RestParameter();
	void Jamp(const Vector2& dir);

	Vector2& GetMoveDir();
	Vector3 GetWorldPosition();
	Transform& GetTransform();
	bool& GetIsCanMove();
	bool& GetIsMoving();
	bool& GetIsBuilding();
	bool& GetIsJumping();
	bool& GetIsGrounded();
	bool GetIsEndTurn();
	bool GetIsAlive() const;
	float GetMoveTimer() const;
	void SetMoveDir(const Vector2& dir);
	void SetIsBuilding(bool set);
	void SetGrounded(bool set);
	void SetAlpha(float alpha);
	void SetAlive(bool set);
	void SetMap(std::vector<std::vector<uint32_t>>* floor, std::vector<std::vector<uint32_t>>* wall);

protected:
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
	bool isReqestBuilding_;
	bool isJumping_;
	bool isGrounded_;
	bool isAlive_;
	float moveTimer_;
	float maxMoveTimer_;
	float shotPower_;
	Vector2 moveDir_;

	float alpha_;

	std::vector<std::vector<uint32_t>>* wallMap_;
	std::vector<std::vector<uint32_t>>* floorMap_;
};