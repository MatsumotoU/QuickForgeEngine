#pragma once
#include "Base/EngineCore.h"
#include "Particle/Particle.h"
#include "Data/ParticleData.h"

static inline const uint32_t kMaxParticle = 3000;

class ParticleManager {
public:
	ParticleManager() = default;
	~ParticleManager() = default;
	void Initialize(EngineCore* engineCore,Camera* camera);
	void Update();
	void Draw();

	void ResetParticle();
	void EmitBomb(const Vector3& position,const Vector4& color,int power, float lifeTime, float velocityDecay,int maxParticle);


private:
	EngineCore* engineCore_;
	Camera* camera_;
	Particle particle_;
	std::vector<ParticleData> particleData_;
};