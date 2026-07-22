#pragma once
#include <vector>
#include "../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "memory/SafeVector.h"

namespace QFE::ASSET {
	/// @brief モデルに使用されているテクスチャの名前を保持する構造体
	struct ModelMaterialData {
		std::string textureName;
	};

	/// @brief モデルのメッシュデータを保持する構造体
	struct MeshData {
		SafeVector<VertexData> vertices;// メッシュの頂点データの配列
		SafeVector<uint32_t> indices;// メッシュのインデックスデータの配列
		ModelMaterialData material;// メッシュのマテリアルデータ

		/// @brief SafeVectorが固定長であるため、コンストラクタでサイズを指定する必要がある
		MeshData(size_t verticesSize, size_t indicesSize) : vertices(verticesSize), indices(indicesSize) {}
	};

	/// @brief モデルデータを保持する構造体
	struct ModelData {
		std::string name;// モデルの名前
		SafeVector<MeshData> meshes;// モデルのメッシュデータの配列
	};
}
