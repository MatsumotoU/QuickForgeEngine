#pragma once
#include "Base/EngineCore.h"

class Mole;

class DiggingEffect
{
public:
	DiggingEffect() = default;
	~DiggingEffect() = default;

	void DebugImGui();

	void Initialize(EngineCore* engineCore, Camera* camera);
	void Update(Mole *mole);
	void Draw();

	void SpwnParticle(Mole* mole);
	void ParticleUpdate();

private:
	EngineCore* engineCore_;
	Camera* camera_;

	struct Particle {
		std::unique_ptr<Model> model;
		float lefeTime = 30;
	};

	std::vector<Particle> particle_;
	const int particleMax_ = 20;
	Vector3 velocity_ = { 0.0f / 60.0f,0.5f/60.0f,0.0f / 60.0f };
};
