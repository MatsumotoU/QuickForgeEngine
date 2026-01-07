#include "engine/include/assets/3DModel/ModelVertexResourceManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include <cassert>

void ModelVertexResourceManager::Initialize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
}

void ModelVertexResourceManager::Finalize() {
	modelDatas_.clear();
	modelVertexBuffers_.clear();
}

uint32_t ModelVertexResourceManager::Assign(ID3D12Device* device, const ModelData& modelData, const std::string& modelName) {
	// 蜷後§蜷榊燕縺ｮ繝｢繝・Ν縺梧里縺ｫ蟄伜惠縺吶ｋ蝣ｴ蜷医・縺昴・繝上Φ繝峨Ν繧定ｿ斐☆
    auto it = modelHandleMap_.find(modelName);
    if (it != modelHandleMap_.end()) {
        return it->second;
    }
    
    // 鬆らせ繝舌ャ繝輔ぃ繧剃ｽ懈・
    if (modelData.meshes.empty()) {
        assert(false && "ModelData has no meshes");
        return 0;
    }
    // 繝｡繝・す繝･縺斐→縺ｫ鬆らせ繝舌ャ繝輔ぃ繧剃ｽ懈・
    uint32_t firstHandle = static_cast<uint32_t>(modelVertexBuffers_.size());
    for (const auto& mesh : modelData.meshes) {
        if (mesh.vertices.empty()) {
            assert(false && "Mesh has no vertices");
            continue; // 鬆らせ縺後↑縺・Γ繝・す繝･縺ｯ繧ｹ繧ｭ繝・・
        }
        modelVertexBuffers_.emplace_back();
        modelVertexBuffers_.back().CreateResource(device, static_cast<uint32_t>(mesh.vertices.size()));
        // 鬆らせ繝・・繧ｿ繧偵そ繝・ヨ
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
