#pragma once
#include "Base/EngineCore.h"
#include "Particle/Particle.h"

class Building {
public:
	void Init(EngineCore* engineCore);
	void Update();
	void Draw(Camera* camera);

private:
	Particle buillding_;
	std::vector<Transform> transform_;
	EngineCore* engineCore_;
};