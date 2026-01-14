/**
 * @file ModelVertexResourceManager.cpp
 * @brief 3Dモデルの頂点リソースを管理するクラスの実装
 */

#include "engine/include/assets/3DModel/ModelVertexResourceManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include <cassert>

/** @brief 初期化 */
void ModelVertexResourceManager::Initialize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
}

/** @brief 終了処理 */
void ModelVertexResourceManager::Finalize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
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
        modelVertexBuffers_.back().CreateResource(device, static_cast<uint32_t>(mesh.vertices.size()));
        // 頂点データをセット
        for (size_t i = 0; i < mesh.vertices.size(); ++i) {
            modelVertexBuffers_.back().SetData(static_cast<uint32_t>(i), mesh.vertices[i]);
        }
    }

	modelHandleMap_.insert({ modelName, firstHandle });
    return firstHandle;
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
