#pragma once
#include <vector>
#include "../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "ModelMaterialData.h"
#include "memory/SafeVector.h"

namespace QFE::GRAPHIC::INTERNAL {

	struct MeshData {
		SafeVector<VertexData> vertices;
		ModelMaterialData material;

		MeshData(size_t size) : vertices(size) {}
	};

	struct ModelData {
		SafeVector<MeshData> meshes;
	};

}
