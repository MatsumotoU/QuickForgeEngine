#include "AssimpModelLoader.h"
#include <cassert>

ModelData AssimpModelLoader::LoadModelData(const std::string& directory, const std::string& filename, CoordinateSystem coordinateSystem) {
    ModelData modelData;

    Assimp::Importer importer;
    std::string filepath = directory + "/" + filename;
    const aiScene* scene = importer.ReadFile(
        filepath,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals
    );
    if (!scene || !scene->HasMeshes()) {
        assert(false && "Assimp: モデルの読み込みに失敗しました");
        return modelData;
    }

    for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
        const aiMesh* mesh = scene->mMeshes[meshIdx];
        MeshData meshData;

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
            if (coordinateSystem == COORDINATESYSTEM_HAND_RIGHT) {
                vtx.position.x *= -1.0f;
                vtx.normal.x *= -1.0f;
                vtx.texcoord.y = 1.0f - vtx.texcoord.y;
            }
            tempVertices.push_back(vtx);
        }

        // インデックス（三角形ごとに頂点を詰める）
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            const aiFace& face = mesh->mFaces[i];
            if (face.mNumIndices == 3) {
                if (coordinateSystem == COORDINATESYSTEM_HAND_RIGHT) {
                    meshData.vertices.push_back(tempVertices[face.mIndices[2]]);
                    meshData.vertices.push_back(tempVertices[face.mIndices[1]]);
                    meshData.vertices.push_back(tempVertices[face.mIndices[0]]);
                } else {
                    meshData.vertices.push_back(tempVertices[face.mIndices[0]]);
                    meshData.vertices.push_back(tempVertices[face.mIndices[1]]);
                    meshData.vertices.push_back(tempVertices[face.mIndices[2]]);
                }
            }
        }

        // マテリアル・テクスチャ
        if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString texPath;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
                meshData.material.textureFilePath = directory + "/" + std::string(texPath.C_Str());
            }
            // ここで他のマテリアル情報（色など）も必要なら取得可能
        }

        modelData.meshes.push_back(std::move(meshData));
    }

    return modelData;
}
