#pragma once
#include <vector>
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "ModelMaterialData.h"
#include "engine/include/core/Memory/SafeVector.h"

namespace QFE {

	struct MeshData {
		SafeVector<VertexData> vertices;
		ModelMaterialData material;
	};

	struct ModelData {
		SafeVector<MeshData> meshes;
	};

}
