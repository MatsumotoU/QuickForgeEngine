#pragma once
#include <vector>
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "ModelMaterialData.h"

struct MeshData {
	std::vector<VertexData> vertices;
	ModelMaterialData material;
};

struct ModelData {
	std::vector<MeshData> meshes;
};
