#pragma once
#include "../../../../Engine/Base/EngineCore.h"
#include "../../../../Engine/Colliders/Collider.h"

#include "Ice.h"

class Bullet : public Collider {
public:
	void Initialize(EngineCore* engineCore,const std::string& name);
	void Update();
	void Draw(Camera* camera);

public:
	void ShotBullet(Vector3 position, Vector3 velocity, uint32_t aliveTime,uint32_t attack = 10);
	void OnCollision(const nlohmann::json& otherData) override;

public:
	bool GetIsActive();
	Vector3 GetWorldPosition() override;
	void SetName(const std::string& name);

	Transform transform_;
	Vector3 velocity_;
	std::string name_;
	bool isHoming_;

private:
	bool isActive_;
	uint32_t aliveTime_;
	uint32_t maxAliveTime_;

private:
	EngineCore* engineCore_;
	Model model_;
	Ice ice_;
};