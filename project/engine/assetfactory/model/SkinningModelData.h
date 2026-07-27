#pragma once
#include <vector>
#include <map>
#include "../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "memory/SafeVector.h"
#include "ModelData.h"
#include "math/MathInclude.h"

namespace QFE::ASSET {
	/// @brief モデルの頂点のウェイト情報を保持する構造体
	struct VertexWeightData {
		float32_t weight;
		uint32_t vertexIndex;
	};

	/// @brief モデルのジョイントのウェイト情報を保持する構造体
	struct JointWeightData {
		QFE::MATH::Matrix4x4 inverseBindPoseMatrix; // ジョイントの逆バインドポーズ行列
		SafeVector<VertexWeightData> vertexWeights; // ジョイントに影響を受ける頂点のウェイト情報の配列
		/// @brief SafeVectorが固定長であるため、コンストラクタでサイズを指定する必要がある
		JointWeightData(size_t size) : vertexWeights(size) {}
	};

	/// @brief モデルのメッシュデータを保持する構造体
	struct SkinningMeshData {
		std::map<std::string, JointWeightData> jointWeights; // ジョイント名をキーとしたジョイントのウェイト情報のマップ
		SafeVector<VertexData> vertices;// メッシュの頂点データの配列
		ModelMaterialData material;// メッシュのマテリアルデータ
		/// @brief SafeVectorが固定長であるため、コンストラクタでサイズを指定する必要がある
		SkinningMeshData(size_t size) : vertices(size) {}
	};

	/// @brief モデルデータを保持する構造体
	struct SkinningModelData {
		std::string name;// モデルの名前
		SafeVector<SkinningMeshData> meshes;// モデルのメッシュデータの配列
	};
}