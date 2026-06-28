#pragma once
#include <vector>
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "ModelMaterialData.h"
#include "engine/include/core/Memory/SafeVector.h"
#include "engine/include/core/Math/Matrix/Matrix4x4.h"

namespace QFE {

	struct VertexWeightData {
		float weight;
		uint32_t vertexIndex;
	};

	struct JointWeightData {
		Matrix4x4 inverseBindPoseMatrix;
		SafeVector<VertexWeightData> vertexWeights;
	};

	struct MeshData {
		SafeVector<VertexData> vertices;
		SafeVector<uint32_t> indices;
		ModelMaterialData material;

		MeshData(size_t vertexSize, size_t indexSize) : vertices(vertexSize), indices(indexSize) {}
	};

	struct ModelData {
		SafeVector<MeshData> meshes;
		std::map<std::string, JointWeightData> skinClusterDara;
	};

}
