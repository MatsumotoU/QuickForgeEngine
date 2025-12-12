#include "AssimpModelLoader.h"
#include <cassert>

#ifdef _DEBUG
#include <format>
#include "AppUtility/DebugTool/DebugLog/MyDebugLog.h"
#endif // _DEBUG

void AssimpModelLoader::LoadModelData(const std::string& modelResourceDirectory, const std::string& imageResourceDirectory, const std::string& filename, ModelData& modelData) {
	Assimp::Importer importer;
	std::string filepath = modelResourceDirectory + filename;
	const aiScene* scene = importer.ReadFile(
		filepath,
		aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals
	);
	if (!scene || !scene->HasMeshes()) {
		assert(false && "Faild Loaded Model");
		return;
	}

#ifdef _DEBUG
	DebugLog(std::format("Model Load Success: {}", filepath));
#endif // _DEBUG

	for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
		const aiMesh* mesh = scene->mMeshes[meshIdx];
		MeshData meshData;

#ifdef _DEBUG
		DebugLog(std::format("Loading Mesh {} / {}", meshIdx + 1, scene->mNumMeshes));
		DebugLog(std::format("UVChannel: {}", mesh->GetNumUVChannels()));
		DebugLog(std::format("ColorChannel: {}", mesh->GetNumColorChannels()));
		DebugLog(std::format("NumUVComponents for channel 0: {}", mesh->mNumUVComponents[0]));
#endif // _DEBUG

		// 頂点データ
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

		// インデックス（三角形ごとに頂点を詰める）
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices == 3) {
				meshData.vertices.push_back(tempVertices[face.mIndices[0]]);
				meshData.vertices.push_back(tempVertices[face.mIndices[2]]);
				meshData.vertices.push_back(tempVertices[face.mIndices[1]]);
			}
		}

		// マテリアル・テクスチャ
		if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiString texPath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
				meshData.material.textureFilePath = imageResourceDirectory + std::string(texPath.C_Str());
			}
		}

		modelData.meshes.push_back(std::move(meshData));
	}

	return;
}