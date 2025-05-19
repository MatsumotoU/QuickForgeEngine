#pragma once
#include "../../../../Engine/Base/EngineCore.h"

static inline const uint32_t kParticles = 8;
static inline const uint32_t kMaxFrame = 120;

class DeathParticle {
public:
	void Initialize(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);
	Transform transform_;

public:
	void EmmitParticle(const Vector3& pos);

public:
	bool GetIsActive();

private:
	uint32_t emmitFrame_;
	bool isActive_;
	EngineCore* engineCore_;
	Model model_[kParticles];
	Vector3 moveDir[kParticles];
	Transform particleTransform[kParticles];
};