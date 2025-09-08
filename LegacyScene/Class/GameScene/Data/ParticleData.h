#pragma once
#include "Math/Transform.h"
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector4.h"

struct ParticleData {
	bool isActive;
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
};