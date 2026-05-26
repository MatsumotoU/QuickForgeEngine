#pragma once
#include "Data/ModelData.h"

namespace QFE::PRIMITIVE {
	QFE::MeshData CreatePlane(float width = 1.0f, float height = 1.0f, uint32_t segmentsX = 1, uint32_t segmentsY = 1, bool invertFace = false);
	QFE::MeshData CreateBox(bool invertFace = false);
	QFE::MeshData CreateRing(float innerRadius = 0.5f, float outerRadius = 1.0f, uint32_t segments = 32, bool invertFace = false);
}
