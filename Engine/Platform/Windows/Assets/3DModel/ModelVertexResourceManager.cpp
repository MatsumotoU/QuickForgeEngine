#include "pch.h"
#include "ModelVertexResourceManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include <cassert>

void ModelVertexResourceManager::Initialize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
}

void ModelVertexResourceManager::Finalize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
}

uint32_t ModelVertexResourceManager::Assign(ID3D12Device* device, const ModelData& modelData) {
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

const D3D12_VERTEX_BUFFER_VIEW* ModelVertexResourceManager::GetVertexBufferView(const uint32_t& handle) {
	assert(handle < modelVertexBuffers_.size() && "Model not found");
	return modelVertexBuffers_.at(handle).GetVertexBufferView();
}