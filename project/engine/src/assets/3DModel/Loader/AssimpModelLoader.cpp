#include "engine/include/assets/3DModel/Loader/AssimpModelLoader.h"
#include "engine/include/utility/FileSystems/FileUtility.h"
#include <cassert>
#include <format>
#include "engine/include/core/EngineDefines.h"

#include "engine/include/utility/FileSystems/FileUtility.h"

using namespace QFE;

void AssimpModelLoader::LoadModelData(const std::string& modelResourceDirectory, const std::string& imageResourceDirectory, const std::string& filename, ModelData& modelData) {
	Assimp::Importer importer;
	std::string filepath = modelResourceDirectory + filename;
	
	// ファイルの存在確認
	if (!QFE::FILE::HasFile(filepath)) {
		QFE_LOG(std::format("Model file not found: {}", filepath));
		assert(false && "Model file not found");
		return;
	}

	// Obj形式である場合、mtlもあるか確認する
	if (!QFE::FILE::HasObjModelFiles(modelResourceDirectory, filename)) {
		QFE_REPORT_SYSTEM_ERROR(std::format("MTL file not found for OBJ model: {}", filename), SystemError::Abort);
	}

	// モデルの読み込み
	const aiScene* scene = importer.ReadFile(
		filepath,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals
	);
	if (!scene || !scene->HasMeshes()) {
		assert(false && "Faild Loaded Model");
		return;
	}

	QFE_LOG(std::format("Model Load Success: {}", filepath));

	for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
		const aiMesh* mesh = scene->mMeshes[meshIdx];

		QFE_LOG(std::format("Loading Mesh {} / {}", meshIdx + 1, scene->mNumMeshes));
		QFE_LOG(std::format("UVChannel: {}", mesh->GetNumUVChannels()));
		QFE_LOG(std::format("ColorChannel: {}", mesh->GetNumColorChannels()));
		QFE_LOG(std::format("NumUVComponents for channel 0: {}", mesh->mNumUVComponents[0]));

		// 頂点データの読み込み
		std::vector<VertexData> tempVertices;
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
			VertexData vtx;
			aiVector3D position = mesh->mVertices[i];
			aiVector3D texcoord = mesh->mTextureCoords[0][i];
			aiVector3D normal = mesh->mNormals[i];
			// 頂点データの変換(右手系から左手系への変換)
			vtx.position = { -position.x, position.y, position.z };
			vtx.texcoord = { texcoord.x, texcoord.y };
			vtx.normal = { -normal.x, normal.y, normal.z };
			tempVertices.push_back(vtx);
		}

		// インデックスデータの読み込み
		std::vector<uint32_t> tempIndices;
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j) {
				unsigned int vertexIndex = face.mIndices[j];
				tempIndices.push_back(vertexIndex);
			}
		}

		// メッシュデータの初期化
		MeshData meshData(tempVertices.size(), tempIndices.size());
		std::copy(tempVertices.begin(), tempVertices.end(), meshData.vertices.begin());
		std::copy(tempIndices.begin(), tempIndices.end(), meshData.indices.begin());

		// マテリアルとテクスチャの読み込み
		if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiString texPath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
				meshData.material.textureFilePath = imageResourceDirectory + std::string(texPath.C_Str());
				QFE_LOG(std::format("Loaded diffuse texture for mesh {}: {}", meshIdx, meshData.material.textureFilePath));

			} else {
				meshData.material.textureFilePath = "";
				QFE_LOG(std::format("No diffuse texture found for mesh {}. Setting empty texture path.", meshIdx));
			}
		}

		modelData.meshes.push_back(std::move(meshData));
	}

	return;
}
