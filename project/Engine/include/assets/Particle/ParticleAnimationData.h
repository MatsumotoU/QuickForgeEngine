#pragma once
#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/Math/Vector/Vector4.h"

#include <vector>

struct ParticleData {
	Transform transform;
	Vector4 color;
};

struct ParticlesData {
	std::vector<ParticleData> particles;
};
