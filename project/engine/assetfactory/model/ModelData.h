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
		ModelMaterialData material;// メッシュのマテリアルデータ

		/// @brief SafeVectorが固定長であるため、コンストラクタでサイズを指定する必要がある
		MeshData(size_t size) : vertices(size) {}
	};

	/// @brief モデルデータを保持する構造体
	struct ModelData {
		std::string name;// モデルの名前
		SafeVector<MeshData> meshes;// モデルのメッシュデータの配列
	};
}
