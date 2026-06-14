#pragma once
#include <vector>
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "ModelMaterialData.h"
#include "engine/include/core/Memory/SafeVector.h"

namespace QFE {

	struct MeshData {
		SafeVector<VertexData> vertices;
		SafeVector<uint32_t> indices;
		ModelMaterialData material;

		MeshData(size_t vertexSize, size_t indexSize) : vertices(vertexSize), indices(indexSize) {}
	};

	struct ModelData {
		SafeVector<MeshData> meshes;
	};

}
