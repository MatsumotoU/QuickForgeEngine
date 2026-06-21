#pragma once
#include "common/ModelData.h"

namespace QFE::GRAPHIC {
	MeshData CreatePlane(float width = 1.0f, float height = 1.0f, uint32_t segmentsX = 1, uint32_t segmentsY = 1, bool invertFace = false);
	MeshData CreateBox(bool invertFace = false);
	MeshData CreateRing(float innerRadius = 0.5f, float outerRadius = 1.0f, uint32_t segments = 32, bool invertFace = false);
}
