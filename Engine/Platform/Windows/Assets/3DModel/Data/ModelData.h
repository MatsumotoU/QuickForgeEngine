#pragma once
#include <vector>
#include "Graphic/ShaderBuffer/Data/VerTexData.h"
#include "ModelMaterialData.h"

struct MeshData {
	std::vector<VertexData> vertices;
	ModelMaterialData material;
};

struct ModelData {
	std::vector<MeshData> meshes;
};