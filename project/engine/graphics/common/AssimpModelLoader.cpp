#include "AssimpModelLoader.h"
#include <cassert>
#include <format>

#include "EngineDefines.h"
#include "file/FileUtility.h"

using namespace QFE::GRAPHIC::INTERNAL;

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
			vtx.position.x = mesh->mVertices[i].x;
			vtx.position.y = mesh->mVertices[i].y;
			vtx.position.z = mesh->mVertices[i].z;
			vtx.position.w = 1.0f;

			if (mesh->HasTextureCoords(0)) {
				vtx.texcoord.x = mesh->mTextureCoords[0][i].x;
				vtx.texcoord.y = mesh->mTextureCoords[0][i].y;
			} else {
				vtx.texcoord.x = 0.0f;
				vtx.texcoord.y = 0.0f;
			}
			if (mesh->HasNormals()) {
				vtx.normal.x = mesh->mNormals[i].x;
				vtx.normal.y = mesh->mNormals[i].y;
				vtx.normal.z = mesh->mNormals[i].z;
			} else {
				vtx.normal.x = 0.0f;
				vtx.normal.y = 0.0f;
				vtx.normal.z = 1.0f;
			}
			vtx.position.x *= -1.0f;
			vtx.normal.x *= -1.0f;
			vtx.texcoord.y = vtx.texcoord.y;

			tempVertices.push_back(vtx);
		}

		// メッシュデータの初期化
		MeshData meshData(tempVertices.size());

		// 面データの読み込み（インデックスを使用して頂点を追加）
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices == 3) {
				meshData.vertices.push_back(tempVertices[face.mIndices[0]]);
				meshData.vertices.push_back(tempVertices[face.mIndices[2]]);
				meshData.vertices.push_back(tempVertices[face.mIndices[1]]);
			}
		}

		// 繝槭ユ繝ｪ繧｢繝ｫ繝ｻ繝・け繧ｹ繝√Ε
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
