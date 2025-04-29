#pragma once
#include <vector>
#include "../Math/VerTexData.h"
#include "../Math/MaterialData.h"

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};