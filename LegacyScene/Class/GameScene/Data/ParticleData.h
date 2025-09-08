#pragma once
#include "Math/Transform.h"
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector4.h"

struct ParticleData {
	bool isActive;
	bool isGravity;
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float velocityDecay;
};