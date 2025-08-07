#pragma once
#include <vector>
#include "Shaders/StructsForGpu/VerTexData.h"
#include "ModelMaterialData.h"

enum CoordinateSystem
{
	COORDINATESYSTEM_HAND_RIGHT,
	COORDINATESYSTEM_HAND_LEFT,
};

struct MeshData {
	std::vector<VertexData> vertices;
	ModelMaterialData material;
};

struct ModelData {
	std::vector<MeshData> meshes;
};