#include "AssimpModelLoader.h"
#include <cassert>
#include <format>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/anim.h>

#include <map>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <limits>
#include <cmath>
#include <unordered_map>

#include "EngineDefines.h"
#include "file/FileUtility.h"

using namespace QFE::ASSET;

namespace QFE::ASSET {
	struct AnimatedGlbScene {
		struct Influence {
			const aiNode* node = nullptr;
			aiMatrix4x4 offset;
			float weight = 0.0f;
		};
		struct MeshInstance {
			const aiNode* node = nullptr;
			const aiMesh* mesh = nullptr;
			std::vector<std::vector<Influence>> influences;
		};
		std::unique_ptr<Assimp::Importer> importer;
		const aiScene* scene = nullptr;
		std::vector<MeshInstance> instances;
		std::unordered_map<std::string, const aiNode*> nodes;
	};
}

namespace {
	std::string ClipName(const aiAnimation* animation, unsigned int index) {
		return animation->mName.length > 0 ? animation->mName.C_Str() : "Animation " + std::to_string(index);
	}

	void CopyGlbMaterial(const aiScene* scene, const aiMesh* mesh,
		const std::string& filePath, ModelMaterialData& output, bool firstMesh) {
		if (mesh->mMaterialIndex >= scene->mNumMaterials) return;
		const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		if (firstMesh) {
			aiColor4D color;
			if (material->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS ||
				material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
				output.baseColor = { color.r, color.g, color.b, color.a };
			}
		}
		if (!output.textureName.empty()) return;
		aiString texturePath;
		if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) != AI_SUCCESS &&
			material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) != AI_SUCCESS) return;
		const aiTexture* embedded = scene->GetEmbeddedTexture(texturePath.C_Str());
		if (embedded && embedded->mHeight == 0) {
			output.textureName = filePath + "#" + texturePath.C_Str();
			const auto* begin = reinterpret_cast<const uint8_t*>(embedded->pcData);
			output.embeddedTextureData.assign(begin, begin + embedded->mWidth);
		} else if (!embedded) {
			output.textureName = (std::filesystem::path(filePath).parent_path() /
				std::filesystem::path(texturePath.C_Str())).lexically_normal().generic_string();
		}
	}

	template<class Key, class Value, class Interpolate>
	Value SampleKeys(const Key* keys, unsigned int count, double tick, Value fallback, Interpolate interpolate) {
		if (!count) return fallback;
		if (count == 1 || tick <= keys[0].mTime) return keys[0].mValue;
		for (unsigned int i = 0; i + 1 < count; ++i) {
			if (tick <= keys[i + 1].mTime) {
				const double span = keys[i + 1].mTime - keys[i].mTime;
				const float factor = span > 0.0 ? static_cast<float>((tick - keys[i].mTime) / span) : 0.0f;
				return interpolate(keys[i].mValue, keys[i + 1].mValue, factor);
			}
		}
		return keys[count - 1].mValue;
	}

	aiMatrix4x4 AnimatedLocal(const aiNode* node, const aiNodeAnim* channel, double tick) {
		if (!channel) return node->mTransformation;
		aiVector3D scale, position;
		aiQuaternion rotation;
		node->mTransformation.Decompose(scale, rotation, position);
		position = SampleKeys(channel->mPositionKeys, channel->mNumPositionKeys, tick, position,
			[](const aiVector3D& a, const aiVector3D& b, float t) { return a + (b - a) * t; });
		scale = SampleKeys(channel->mScalingKeys, channel->mNumScalingKeys, tick, scale,
			[](const aiVector3D& a, const aiVector3D& b, float t) { return a + (b - a) * t; });
		rotation = SampleKeys(channel->mRotationKeys, channel->mNumRotationKeys, tick, rotation,
			[](const aiQuaternion& a, const aiQuaternion& b, float t) {
				aiQuaternion result;
				aiQuaternion::Interpolate(result, a, b, t);
				return result.Normalize();
			});
		return aiMatrix4x4(scale, rotation, position);
	}

	void GatherAnimatedNodes(AnimatedGlbScene& asset, const aiNode* node) {
		asset.nodes.try_emplace(node->mName.C_Str(), node);
		for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
			const aiMesh* mesh = asset.scene->mMeshes[node->mMeshes[i]];
			AnimatedGlbScene::MeshInstance instance;
			instance.node = node;
			instance.mesh = mesh;
			instance.influences.resize(mesh->mNumVertices);
			asset.instances.push_back(std::move(instance));
		}
		for (unsigned int i = 0; i < node->mNumChildren; ++i) GatherAnimatedNodes(asset, node->mChildren[i]);
	}

	bool PrepareAnimatedGlb(AnimatedGlbScene& asset) {
		if (!asset.scene || !asset.scene->mRootNode) return false;
		GatherAnimatedNodes(asset, asset.scene->mRootNode);
		for (auto& instance : asset.instances) {
			for (unsigned int i = 0; i < instance.mesh->mNumBones; ++i) {
				const aiBone* bone = instance.mesh->mBones[i];
				const auto found = asset.nodes.find(bone->mName.C_Str());
				if (found == asset.nodes.end()) continue;
				for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
					const auto& weight = bone->mWeights[j];
					if (weight.mVertexId < instance.influences.size()) {
						instance.influences[weight.mVertexId].push_back({ found->second, bone->mOffsetMatrix, weight.mWeight });
					}
				}
			}
		}
		return !asset.instances.empty();
	}

	aiVector3D TransformNormal(const aiMatrix4x4& matrix, const aiVector3D& normal) {
		aiMatrix4x4 inverseTranspose = matrix;
		inverseTranspose.Inverse().Transpose();
		return { inverseTranspose.a1 * normal.x + inverseTranspose.a2 * normal.y + inverseTranspose.a3 * normal.z,
			inverseTranspose.b1 * normal.x + inverseTranspose.b2 * normal.y + inverseTranspose.b3 * normal.z,
			inverseTranspose.c1 * normal.x + inverseTranspose.c2 * normal.y + inverseTranspose.c3 * normal.z };
	}

	void SampleAnimatedVertices(const AnimatedGlbScene& asset, const aiAnimation* clip,
		double tick, std::vector<VertexData>& output) {
		std::unordered_map<std::string, const aiNodeAnim*> channels;
		if (clip) for (unsigned int i = 0; i < clip->mNumChannels; ++i)
			channels[clip->mChannels[i]->mNodeName.C_Str()] = clip->mChannels[i];
		std::unordered_map<const aiNode*, aiMatrix4x4> globals;
		auto visit = [&](auto&& self, const aiNode* node, const aiMatrix4x4& parent) -> void {
			const auto found = channels.find(node->mName.C_Str());
			const aiMatrix4x4 global = parent * AnimatedLocal(node,
				found == channels.end() ? nullptr : found->second, tick);
			globals.emplace(node, global);
			for (unsigned int i = 0; i < node->mNumChildren; ++i) self(self, node->mChildren[i], global);
		};
		visit(visit, asset.scene->mRootNode, aiMatrix4x4());
		output.clear();
		for (const auto& instance : asset.instances) {
			const aiMesh* mesh = instance.mesh;
			for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
				const aiVector3D sourceNormal = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0, 0, 1);
				aiVector3D position(0, 0, 0), normal(0, 0, 0);
				float totalWeight = 0.0f;
				for (const auto& influence : instance.influences[i]) {
					const aiMatrix4x4 matrix = globals.at(influence.node) * influence.offset;
					position += (matrix * mesh->mVertices[i]) * influence.weight;
					normal += TransformNormal(matrix, sourceNormal) * influence.weight;
					totalWeight += influence.weight;
				}
				const aiMatrix4x4& rigid = globals.at(instance.node);
				if (totalWeight < 1.0f) {
					const float remaining = 1.0f - totalWeight;
					position += (rigid * mesh->mVertices[i]) * remaining;
					normal += TransformNormal(rigid, sourceNormal) * remaining;
				}
				if (normal.SquareLength() > 0.0f) normal.Normalize();
				VertexData vertex{};
				vertex.position = { position.x, position.y, position.z, 1.0f };
				vertex.normal = { normal.x, normal.y, normal.z };
				if (mesh->HasTextureCoords(0)) {
					vertex.texcoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
				}
				output.push_back(vertex);
			}
		}
	}

	bool LoadAnimatedGlbScene(const AnimatedGlbScene& asset, const std::string& filePath, ModelData& modelData) {
		uint64_t vertexCount = 0, indexCount = 0;
		for (const auto& instance : asset.instances) {
			vertexCount += instance.mesh->mNumVertices;
			for (unsigned int i = 0; i < instance.mesh->mNumFaces; ++i)
				if (instance.mesh->mFaces[i].mNumIndices == 3) indexCount += 3;
		}
		if (!vertexCount || !indexCount || vertexCount > (std::numeric_limits<uint32_t>::max)()) return false;
		MeshData combined(static_cast<size_t>(vertexCount), static_cast<size_t>(indexCount));
		std::vector<VertexData> vertices;
		SampleAnimatedVertices(asset, nullptr, 0.0, vertices);
		combined.vertices.GetInternalVector() = std::move(vertices);
		uint32_t base = 0;
		for (size_t instanceIndex = 0; instanceIndex < asset.instances.size(); ++instanceIndex) {
			const aiMesh* mesh = asset.instances[instanceIndex].mesh;
			for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
				const aiFace& face = mesh->mFaces[i];
				if (face.mNumIndices != 3) continue;
				for (unsigned int j = 0; j < 3; ++j) combined.indices.push_back(base + face.mIndices[j]);
			}
			CopyGlbMaterial(asset.scene, mesh, filePath, combined.material, instanceIndex == 0);
			base += mesh->mNumVertices;
		}
		if (asset.scene->mNumMaterials > 1) QFE_LOG("GLB has multiple materials; the current renderer uses one texture for the combined mesh.");
		modelData.meshes.clear();
		modelData.meshes.push_back(std::move(combined));
		return true;
	}

	bool LoadStaticGlbScene(const aiScene* scene, const std::string& filePath, ModelData& modelData) {
		uint64_t vertexCount = 0;
		uint64_t indexCount = 0;
		for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
			const aiMesh* mesh = scene->mMeshes[meshIndex];
			vertexCount += mesh->mNumVertices;
			for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
				if (mesh->mFaces[faceIndex].mNumIndices == 3) indexCount += 3;
			}
		}
		if (vertexCount == 0 || indexCount == 0 ||
			vertexCount > (std::numeric_limits<uint32_t>::max)()) return false;

		// The current renderer draws one vertex/index buffer per model. Assimp has
		// already baked the GLB node transforms, so concatenate every static mesh.
		MeshData combined(static_cast<size_t>(vertexCount), static_cast<size_t>(indexCount));
		for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
			const aiMesh* mesh = scene->mMeshes[meshIndex];
			const uint32_t baseVertex = static_cast<uint32_t>(combined.vertices.size());
			for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
				VertexData vertex;
				const aiVector3D& position = mesh->mVertices[vertexIndex];
				vertex.position = { position.x, position.y, position.z, 1.0f };
				if (mesh->HasTextureCoords(0)) {
					const aiVector3D& uv = mesh->mTextureCoords[0][vertexIndex];
					vertex.texcoord = { uv.x, uv.y };
				}
				if (mesh->HasNormals()) {
					const aiVector3D& normal = mesh->mNormals[vertexIndex];
					vertex.normal = { normal.x, normal.y, normal.z };
				}
				combined.vertices.push_back(vertex);
			}
			for (unsigned int faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
				const aiFace& face = mesh->mFaces[faceIndex];
				if (face.mNumIndices != 3) continue;
				for (unsigned int corner = 0; corner < 3; ++corner) {
					combined.indices.push_back(baseVertex + face.mIndices[corner]);
				}
			}

			if (mesh->mMaterialIndex >= scene->mNumMaterials) continue;
			const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			if (meshIndex == 0) {
				aiColor4D color;
				if (material->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS ||
					material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
					combined.material.baseColor = { color.r, color.g, color.b, color.a };
				}
			}
			if (!combined.material.textureName.empty()) continue;
			aiString texturePath;
			if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath) != AI_SUCCESS &&
				material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) != AI_SUCCESS) continue;
			if (const aiTexture* embedded = scene->GetEmbeddedTexture(texturePath.C_Str());
				embedded != nullptr && embedded->mHeight == 0) {
				combined.material.textureName = filePath + "#" + texturePath.C_Str();
				const auto* begin = reinterpret_cast<const uint8_t*>(embedded->pcData);
				combined.material.embeddedTextureData.assign(begin, begin + embedded->mWidth);
			} else if (embedded == nullptr) {
				combined.material.textureName =
					(std::filesystem::path(filePath).parent_path() /
						std::filesystem::path(texturePath.C_Str())).lexically_normal().generic_string();
			}
		}
		if (scene->mNumMaterials > 1) {
			QFE_LOG("GLB has multiple materials; the current renderer uses one texture for the combined mesh.");
		}
		modelData.meshes.clear();
		modelData.meshes.push_back(std::move(combined));
		return true;
	}
}

void AssimpModelLoader::Initialize() {
	// キャッシュを初期化する
	modelCache.clear();
	animatedGlbCache.clear();
	invalidModelData = ModelData(); // 無効なモデルデータを初期化
	QFE_LOG("AssimpModelLoader initialized, model cache cleared.");
}

bool QFE::ASSET::AssimpModelLoader::LoadModel(const std::string& filePath, ModelData& modelData) {
	if (IsModelCached(filePath)) {
		modelData = modelCache[filePath];
		return true;
	} else {
		if (LoadModelData(filePath, modelData)) {
			modelCache[filePath] = modelData; // キャッシュに保存
			return true;
		} else {
			QFE_LOG(std::format("Failed to load model: {}", filePath));
			return false;
		}
	}
	return false;
}

ModelData& AssimpModelLoader::LoadModel(const std::string& filePath) {
	// キャッシュにモデルデータが存在する場合はキャッシュから返す
	if (IsModelCached(filePath)) {
		return modelCache[filePath];
	} else {
		LoadModelData(filePath, modelCache[filePath]);
		return modelCache[filePath];
	}
}

SkinningModelData& QFE::ASSET::AssimpModelLoader::LoadSkinningModel(const std::string& filePath) {
	// キャッシュにスキニングモデルデータが存在する場合はキャッシュから返す
	if (skinningModelCache.find(filePath) != skinningModelCache.end()) {
		return skinningModelCache[filePath];
	} else {
		LoadSkinningModelData(filePath, skinningModelCache[filePath]);
		return skinningModelCache[filePath];
	}
}

ModelData& AssimpModelLoader::ForceLoadModel(const std::string& filePath) {
	// キャッシュを無視した場合ログを出力する
	if (IsModelCached(filePath)) {
		QFE_LOG(std::format("Force loading model, but it is already cached: {}", filePath));
	}

	// キャッシュを無視して新たにモデルデータを読み込む,キャッシュに存在する場合は上書きする
	animatedGlbCache.erase(filePath);
	LoadModelData(filePath, modelCache[filePath]);
	return modelCache[filePath];
}

std::vector<std::string> AssimpModelLoader::GetGlbAnimationNames(const std::string& filePath) const {
	std::vector<std::string> names;
	const auto found = animatedGlbCache.find(filePath);
	if (found == animatedGlbCache.end()) return names;
	const aiScene* scene = found->second->scene;
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i) names.push_back(ClipName(scene->mAnimations[i], i));
	return names;
}

double AssimpModelLoader::GetGlbAnimationDuration(const std::string& filePath, const std::string& clipName) const {
	const auto found = animatedGlbCache.find(filePath);
	if (found == animatedGlbCache.end()) return 0.0;
	const aiScene* scene = found->second->scene;
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
		const aiAnimation* clip = scene->mAnimations[i];
		if (clipName.empty() && i == 0 || clipName == ClipName(clip, i)) {
			return clip->mDuration / (clip->mTicksPerSecond > 0.0 ? clip->mTicksPerSecond : 1.0);
		}
	}
	return 0.0;
}

bool AssimpModelLoader::SampleGlbAnimation(const std::string& filePath, const std::string& clipName,
	double timeSeconds, std::vector<VertexData>& outVertices) const {
	const auto found = animatedGlbCache.find(filePath);
	if (found == animatedGlbCache.end()) return false;
	const aiScene* scene = found->second->scene;
	if (!scene->mNumAnimations) return false;
	const aiAnimation* clip = nullptr;
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
		if (clipName.empty() && i == 0 || clipName == ClipName(scene->mAnimations[i], i)) {
			clip = scene->mAnimations[i];
			break;
		}
	}
	if (!clip) return false;
	const double ticksPerSecond = clip->mTicksPerSecond > 0.0 ? clip->mTicksPerSecond : 1.0;
	const double tick = std::clamp(timeSeconds * ticksPerSecond, 0.0, clip->mDuration);
	SampleAnimatedVertices(*found->second, clip, tick, outVertices);
	return true;
}

bool AssimpModelLoader::IsModelCached(const std::string& filePath) const {
	// キャッシュにモデルデータが存在するかどうかを確認
	if (modelCache.find(filePath) != modelCache.end()) {
		QFE_LOG(std::format("Model found in cache: {}", filePath));
		return true;
	} else {
		QFE_LOG(std::format("Model not found in cache: {}", filePath));
		return false;
	}
}

bool AssimpModelLoader::LoadModelData(const std::string& filePath, ModelData& modelData) {
	auto importer = std::make_unique<Assimp::Importer>();
	// ファイルの存在確認
	if (!QFE::FILE::HasFile(filePath)) {
		QFE_LOG(std::format("Model file not found: {}", filePath));
		assert(false && "Model file not found");
		return false;
	}

	// モデルの名前を設定
	modelData.name = QFE::FILE::GetFileName(filePath);

	// モデルの読み込み
	std::string extension = std::filesystem::path(filePath).extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(),
		[](unsigned char value) { return static_cast<char>(std::tolower(value)); });
	const bool isGlb = extension == ".glb";
	const aiScene* scene = importer->ReadFile(
		filePath,
		isGlb
			? (aiProcess_Triangulate | aiProcess_GenNormals |
				aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder)
			: (aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals));
	if (!scene || !scene->HasMeshes()) {
		assert(false && "Failed to load model");
		return false;
	}

	QFE_LOG(std::format("Model Load Success: {}", filePath));
	if (isGlb) {
		if (scene->HasAnimations()) {
			auto asset = std::make_shared<AnimatedGlbScene>();
			asset->scene = scene;
			asset->importer = std::move(importer);
			if (!PrepareAnimatedGlb(*asset) || !LoadAnimatedGlbScene(*asset, filePath, modelData)) return false;
			animatedGlbCache[filePath] = std::move(asset);
			return true;
		}
		scene = importer->ApplyPostProcessing(aiProcess_PreTransformVertices);
		return scene && LoadStaticGlbScene(scene, filePath, modelData);
	}

	for (unsigned int meshIdx = 0; meshIdx < scene->mNumMeshes; ++meshIdx) {
		const aiMesh* mesh = scene->mMeshes[meshIdx];

		QFE_LOG(std::format("Loading Mesh {} / {}", meshIdx + 1, scene->mNumMeshes));
		QFE_LOG(std::format("UVChannel: {}", mesh->GetNumUVChannels()));
		QFE_LOG(std::format("ColorChannel: {}", mesh->GetNumColorChannels()));
		QFE_LOG(std::format("NumUVComponents for channel 0: {}", mesh->mNumUVComponents[0]));

		// 1) 元頂点配列を作る（mesh->mNumVertices 個）
		std::vector<VertexData> tempVertices;
		tempVertices.reserve(mesh->mNumVertices);
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
			tempVertices.push_back(vtx);
		}

		// 2) MeshData を頂点数とインデックス数で初期化
		MeshData meshData(mesh->mNumVertices, mesh->mNumFaces * 3);

		// 3) tempVertices を meshData.vertices にコピー
		{
			auto& dst = meshData.vertices.GetInternalVector();
			dst = std::move(tempVertices); // 所有権を移す（コピーでも可）
		}

		// 4) faces からインデックス配列を作成（Assimp の face.mIndices をそのまま使用）
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices == 3) {
				meshData.indices.push_back(static_cast<uint32_t>(face.mIndices[0]));
				meshData.indices.push_back(static_cast<uint32_t>(face.mIndices[1]));
				meshData.indices.push_back(static_cast<uint32_t>(face.mIndices[2]));
			}
		}

		// 5) マテリアルの読み込み（既存コード）
		if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiString texPath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
				meshData.material.textureName = std::string(texPath.C_Str());
				QFE_LOG(std::format("Loaded diffuse texture for mesh {}: {}", meshIdx, meshData.material.textureName));
			} else {
				meshData.material.textureName = "";
				QFE_LOG(std::format("No diffuse texture found for mesh {}. Setting empty texture path.", meshIdx));
			}
		}

		modelData.meshes.push_back(std::move(meshData));
	}

	return true;
}

bool QFE::ASSET::AssimpModelLoader::LoadSkinningModelData(const std::string& filePath, SkinningModelData& skinningModelData) {
	Assimp::Importer importer;
	// ファイルの存在確認
	if (!QFE::FILE::HasFile(filePath)) {
		QFE_LOG(std::format("Model file not found: {}", filePath));
		assert(false && "Model file not found");
		return false;
	}

	// モデルの名前を設定
	skinningModelData.name = QFE::FILE::GetFileName(filePath);

	// モデルの読み込み
	const aiScene* scene = importer.ReadFile(
		filePath,
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GenNormals
	);
	if (!scene || !scene->HasMeshes()) {
		assert(false && "Failed to load model");
		return false;
	}

	QFE_LOG(std::format("Model Load Success: {}", filePath));

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
			tempVertices.push_back(vtx);
		}

		SkinningMeshData meshData(mesh->mNumFaces * 3);

		// 面データの読み込み
		for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
			const aiFace& face = mesh->mFaces[i];
			if (face.mNumIndices == 3) {
				meshData.vertices.push_back(tempVertices[face.mIndices[0]]);
				meshData.vertices.push_back(tempVertices[face.mIndices[1]]);
				meshData.vertices.push_back(tempVertices[face.mIndices[2]]);
			}
		}

		//Skinning情報の読み込み
		for(uint32_t boneIndex = 0;boneIndex<mesh->mNumBones;++boneIndex) {
			const aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName(bone->mName.C_Str());
			auto [it, inserted] = meshData.jointWeights.try_emplace(jointName, bone->mNumWeights);
			JointWeightData& jointWeightData = it->second;

			aiMatrix4x4 bindPoseMatrix = bone->mOffsetMatrix;
			aiVector3D scale, translate;
			aiQuaternion rotation;
			bindPoseMatrix.Decompose(scale, rotation, translate);
			QFE::MATH::Matrix4x4 bindPose = 
				QFE::MATH::Matrix4x4::MakeAffineMatrix(
					{ translate.x, translate.y, translate.z }, 
					{ rotation.x, rotation.y, rotation.z, rotation.w }, 
					{ scale.x, scale.y, scale.z }
				);
			jointWeightData.inverseBindPoseMatrix = bindPose.Inverse();
			for(uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				const aiVertexWeight& vertexWeight = bone->mWeights[weightIndex];
				VertexWeightData vtxWeight(bone->mNumWeights);
				vtxWeight.vertexIndex = vertexWeight.mVertexId;
				vtxWeight.weight = vertexWeight.mWeight;
				jointWeightData.vertexWeights.push_back(vtxWeight);
			}
		}

		// マテリアルの読み込み
		if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiString texPath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
				meshData.material.textureName = std::string(texPath.C_Str());
				QFE_LOG(std::format("Loaded diffuse texture for mesh {}: {}", meshIdx, meshData.material.textureName));

			} else {
				meshData.material.textureName = "";
				QFE_LOG(std::format("No diffuse texture found for mesh {}. Setting empty texture path.", meshIdx));
			}
		}

		skinningModelData.meshes.push_back(std::move(meshData));
	}

	return true;
}
