#pragma once
#include "Base/EngineCore.h"
#include "Particle/Particle.h"

class ExprotionParticle {
public:
	void Init(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

	void Generate(const Vector3& position);

private:
	bool isActive_;
	Particle particle;
	std::vector<Transform> transform_;
	EngineCore* engineCore_;
	int life_;
};