/**
 * @file ModelVertexResourceManager.cpp
 * @brief 3Dモデルの頂点リソースを管理するクラスの実装
 */

#include "engine/include/assets/3DModel/ModelVertexResourceManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include <cassert>

#include "engine/include/assets/3DModel/PrimitiveVertices.h"

using namespace QFE;

/** @brief 初期化 */
void ModelVertexResourceManager::Initialize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
	modelHandleMap_.clear();
}

/** @brief 終了処理 */
void ModelVertexResourceManager::Finalize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
	modelHandleMap_.clear();
}

/**
 * @brief モデルデータを割り当てて頂点バッファを作成する
 * TODO: 1つのモデルが複数のメッシュを持つ場合、連続するハンドルが割り当てられるが、
 *       外部からはそれらが1つのモデルに属することが分かりにくい設計になっている。
 */
uint32_t ModelVertexResourceManager::Assign(ID3D12Device* device, const ModelData& modelData, const std::string& modelName) {
	// 同じ名前のモデルが既に存在する場合はそのハンドルを返す
    auto it = modelHandleMap_.find(modelName);
    if (it != modelHandleMap_.end()) {
        return it->second;
    }
    
    // 頂点バッファを作成
    if (modelData.meshes.empty()) {
        assert(false && "ModelData has no meshes");
        return 0;
    }
    // メッシュごとに頂点バッファを作成
    uint32_t firstHandle = static_cast<uint32_t>(modelVertexBuffers_.size());
    for (const auto& mesh : modelData.meshes) {
        if (mesh.vertices.empty()) {
            assert(false && "Mesh has no vertices");
            continue; // 頂点がないメッシュはスキップ
        }
        modelVertexBuffers_.emplace_back();
        modelVertexBuffers_.back().CreateResource(device, static_cast<uint32_t>(mesh.vertices.size()), static_cast<uint32_t>(mesh.indices.size()));
        // 頂点データをセット
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            modelVertexBuffers_.back().SetData(static_cast<uint32_t>(i), mesh.vertices[i]);
        }
        // インデックスデータをセット
        for (size_t i = 0; i < mesh.indices.size(); ++i) {
            modelVertexBuffers_.back().SetIndexData(static_cast<uint32_t>(i), mesh.indices[i]);
        }
    }

	modelHandleMap_.insert({ modelName, firstHandle });
    return firstHandle;
}

uint32_t QFE::ModelVertexResourceManager::AssignPlane(ID3D12Device* device, float width, float height, uint32_t segmentsX, uint32_t segmentsY, bool invertFace) {
	// 同じ名前のモデルが既に存在する場合はそのハンドルを返す
	std::string modelName = "Plane_" + std::to_string(width) + "_" + std::to_string(height) + "_" + std::to_string(segmentsX) + "_" + std::to_string(segmentsY) + "_" + std::to_string(invertFace);
	auto it = modelHandleMap_.find(modelName);
    if (it != modelHandleMap_.end()) {
        return it->second;
    }
	// メッシュデータを作成する
	MeshData planeMesh = PRIMITIVE::CreatePlane(width, height, segmentsX, segmentsY, invertFace);
	// 頂点データを割り当てる
	modelVertexBuffers_.emplace_back();
	modelVertexBuffers_.back().CreateResource(device, static_cast<uint32_t>(planeMesh.vertices.size()), static_cast<uint32_t>(planeMesh.indices.size()));
    for (size_t i = 0; i < planeMesh.vertices.size(); ++i) {
        modelVertexBuffers_.back().SetData(static_cast<uint32_t>(i), planeMesh.vertices[i]);
	}
	// インデックスデータをセット
	for (size_t i = 0; i < planeMesh.indices.size(); ++i) {
		modelVertexBuffers_.back().SetIndexData(static_cast<uint32_t>(i), planeMesh.indices[i]);
	}
	// ハンドルを生成してマップに登録
	modelHandleMap_.insert({ modelName, static_cast<uint32_t>(modelVertexBuffers_.size() - 1) });
	// ハンドルを返す
	return static_cast<uint32_t>(modelVertexBuffers_.size() - 1);
}

uint32_t QFE::ModelVertexResourceManager::AssignBox(ID3D12Device* device, bool invertFace)
{
	// 同じ名前のモデルが既に存在する場合はそのハンドルを返す
	std::string modelName = "Box_" + std::to_string(modelVertexBuffers_.size());
	auto it = modelHandleMap_.find(modelName);
	if (it != modelHandleMap_.end()) {
		return it->second;
	}

    // メッシュデータを作成する
	MeshData boxMesh = PRIMITIVE::CreateBox(invertFace);
	
	// 頂点データを割り当てる
	modelVertexBuffers_.emplace_back();
	modelVertexBuffers_.back().CreateResource(device, static_cast<uint32_t>(boxMesh.vertices.size()), static_cast<uint32_t>(boxMesh.indices.size()));
	for (size_t i = 0; i < boxMesh.vertices.size(); ++i) {
		modelVertexBuffers_.back().SetData(static_cast<uint32_t>(i), boxMesh.vertices[i]);
	}
	// インデックスデータをセット
	for (size_t i = 0; i < boxMesh.indices.size(); ++i) {
		modelVertexBuffers_.back().SetIndexData(static_cast<uint32_t>(i), boxMesh.indices[i]);
	}

	// ハンドルを生成してマップに登録
	modelHandleMap_.insert({ modelName, static_cast<uint32_t>(modelVertexBuffers_.size() - 1) });

	// ハンドルを返す
	return static_cast<uint32_t>(modelVertexBuffers_.size() - 1);
}

uint32_t QFE::ModelVertexResourceManager::AssignRing(ID3D12Device* device, float innerRadius, float outerRadius, uint32_t segments, bool invertFace) {
    // 同じ名前のモデルが既に存在する場合はそのハンドルを返す
    std::string modelName = "Ring_" + std::to_string(innerRadius) + "_" + std::to_string(outerRadius) + "_" + std::to_string(segments) + "_" + std::to_string(invertFace);
    auto it = modelHandleMap_.find(modelName);
    if (it != modelHandleMap_.end()) {
        return it->second;
    }
    // メッシュデータを作成する
    MeshData ringMesh = PRIMITIVE::CreateRing(0.5f, 1.0f, 32, invertFace);
    // 頂点データを割り当てる
    modelVertexBuffers_.emplace_back();
    modelVertexBuffers_.back().CreateResource(device, static_cast<uint32_t>(ringMesh.vertices.size()), static_cast<uint32_t>(ringMesh.indices.size()));
    for (size_t i = 0; i < ringMesh.vertices.size(); ++i) {
        modelVertexBuffers_.back().SetData(static_cast<uint32_t>(i), ringMesh.vertices[i]);
    }
    // インデックスデータをセット
    for (size_t i = 0; i < ringMesh.indices.size(); ++i) {
        modelVertexBuffers_.back().SetIndexData(static_cast<uint32_t>(i), ringMesh.indices[i]);
    }
    // ハンドルを生成してマップに登録
    modelHandleMap_.insert({ modelName, static_cast<uint32_t>(modelVertexBuffers_.size() - 1) });
    // ハンドルを返す
    return static_cast<uint32_t>(modelVertexBuffers_.size() - 1);
}

const uint32_t ModelVertexResourceManager::GetVertexBufferCount(uint32_t handle) const {
	assert(handle < modelVertexBuffers_.size() && "Model not found");

	return modelVertexBuffers_.at(handle).GetVertexCount();
}

ID3D12Resource* ModelVertexResourceManager::GetModelVertexBuffer(const uint32_t& handle) {
	assert(handle < modelVertexBuffers_.size() && "Model not found");
	return modelVertexBuffers_.at(handle).GetResource();
}

VertexData* ModelVertexResourceManager::GetModelVertexBufferData(const uint32_t& handle) {
	return modelVertexBuffers_.at(handle).GetData();
}

const D3D12_VERTEX_BUFFER_VIEW* ModelVertexResourceManager::GetVertexBufferView(const uint32_t& handle) {
	assert(handle < modelVertexBuffers_.size() && "Model not found");
	return modelVertexBuffers_.at(handle).GetVertexBufferView();
}

D3D12_INDEX_BUFFER_VIEW QFE::ModelVertexResourceManager::GetIndexBufferView(const uint32_t& handle) {
	return modelVertexBuffers_.at(handle).GetIndexBufferView();
}

uint32_t QFE::ModelVertexResourceManager::GetIndexCount(const uint32_t& handle) const {
	return modelVertexBuffers_.at(handle).GetIndexCount();
}

uint32_t ModelVertexResourceManager::GetModelHandle(const std::string& modelName) const {
    auto it = modelHandleMap_.find(modelName);
    if (it != modelHandleMap_.end()) {
        return it->second;
    }
    assert(false && "Model not found");
    return 0;
}

bool ModelVertexResourceManager::HasModelHandle(const std::string& modelName) const {
    auto it = modelHandleMap_.find(modelName);
    if (it != modelHandleMap_.end()) {
        return true;
    }
    return false;
}
