#pragma once
#include "Core/Math/Transform.h"
#include "Core/Math/Vector/Vector4.h"

#include <vector>

struct ParticleData {
	Transform transform;
	Vector4 color;
};

struct ParticlesData {
	std::vector<ParticleData> particles;
};