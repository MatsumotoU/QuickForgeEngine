#pragma once
#include "engine/include/core/Math/Transform.h"
#include "engine/include/core/Math/Vector/Vector4.h"

#include "engine/include/core/Memory/SafeVector.h"

namespace QFE {

	struct ParticleData {
		EulerTransform transform;
		Vector4 color;
	};

	struct ParticlesData {
		SafeVector<ParticleData> particles;
	};

}
