#pragma once
#include "Base/EngineCore.h"
#include "Particle/Particle.h"
#include "Data/ParticleData.h"

static inline const uint32_t kMaxParticle = 1500;
static inline const uint32_t kMaxOnomatope = 100;

class ParticleManager {
public:
	ParticleManager() = default;
	~ParticleManager() = default;
	void Initialize(EngineCore* engineCore,Camera* camera);
	void Update();
	void Draw();

	void ResetParticle();
	void EmitBomb(const Vector3& position,const Vector4& color,int power, float lifeTime, float velocityDecay,int maxParticle);
	void EmitBako(const Vector3& position, const Vector4& color, int power, float lifeTime);
	void EmitDoka(const Vector3& position, const Vector4& color, int power, float lifeTime);
	void EmitZudon(const Vector3& position, const Vector4& color, int power, float lifeTime);
	void EmitByon(const Vector3& position, const Vector4& color, int power, float lifeTime);

private:
	EngineCore* engineCore_;
	Camera* camera_;
	Particle particle_;
	Particle bako_;
	Particle doka_;
	Particle zudon_;
	Particle byon_;
	std::vector<ParticleData> particleData_;
	std::vector<ParticleData> bakoData_;
	std::vector<ParticleData> dokaData_;
	std::vector<ParticleData> zudonData_;
	std::vector<ParticleData> byonData_;

};