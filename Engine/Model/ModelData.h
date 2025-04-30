#pragma once
#include <vector>
#include "../Math/VerTexData.h"
#include "../Math/MaterialData.h"

enum CoordinateSystem
{
	COORDINATESYSTEM_HAND_RIGHT,
	COORDINATESYSTEM_HAND_LEFT,
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};