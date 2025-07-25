#pragma once
#include "../../../../Engine/Base/EngineCore.h"

#include <map>
#include <functional>

enum class EnemyBahaviorType {
	kMove,
	kDeath,
};

class Enemy {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	Transform transform_;

	bool GetIsAlive() const { return isAlive_; }

private:
	void Move();
	void Death();

public:
	float frameCount_;
	Vector3 velocity_;
	Vector3 acceleration_;

	EnemyBahaviorType bahaviorType_;
	std::map<EnemyBahaviorType, std::function<void()>> bahaviorTable_;

private:
	Model model_;
	EngineCore* engineCore_;

	bool isAlive_;
	uint32_t deathAnimationFrameCount_;

};