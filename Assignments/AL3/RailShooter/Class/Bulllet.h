#pragma once
#include "../../../../Engine/Base/EngineCore.h"
#include "../../../../Engine/Colliders/Collider.h"

class Bullet : public Collider {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

public:
	void ShotBullet(Vector3 position, Vector3 velocity, uint32_t aliveTime);
	void OnCollision() override;

public:
	bool GetIsActive();
	Vector3 GetWorldPosition() override;
	void SetName(const std::string& name);

	Transform transform_;
	Vector3 velocity_;
	std::string name_;

private:
	bool isActive_;
	uint32_t aliveTime_;
	uint32_t maxAliveTime_;

private:
	EngineCore* engineCore_;
	Model model_;

};