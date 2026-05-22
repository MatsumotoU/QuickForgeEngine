#pragma once
#include "Data/ModelData.h"

namespace QFE::PRIMITIVE {
	QFE::MeshData CreateBox(bool invertFace = false);
	QFE::MeshData CreateRing(float innerRadius = 0.5f, float outerRadius = 1.0f, uint32_t segments = 32, bool invertFace = false);
}
