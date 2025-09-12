#include "ModelVertexResourceManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include <cassert>

void ModelVertexResourceManager::Initialize() {
	modelVertexBuffers.clear();
}

void ModelVertexResourceManager::AssignModelVertexBuffer(ID3D12Device* device, const std::string& modelName, const ModelData& modelData) {
	// すでに登録されている場合はスキップ
	if (modelVertexBuffers.find(modelName) != modelVertexBuffers.end()) {
		return;
	}
	// 頂点バッファを作成
	if (modelData.meshes.empty()) {
		assert(false && "ModelData has no meshes");
		return;
	}
	// メッシュごとに頂点バッファを作成
	int meshIndex = 0;
	for (const auto& mesh : modelData.meshes) {
		VertexBuffer<VertexData> vertexBuffer;
		std::string meshName = modelName + "_mesh" + std::to_string(meshIndex++);
		modelVertexBuffers[meshName].CreateResource(device, static_cast<uint32_t>(mesh.vertices.size()));
		// 頂点データをセット
		for (size_t i = 0; i < mesh.vertices.size(); ++i) {
			modelVertexBuffers[meshName].SetData(static_cast<uint32_t>(i), mesh.vertices[i]);
		}
	}
}

ID3D12Resource* ModelVertexResourceManager::GetModelVertexBuffer(const std::string& modelName) {
	assert(modelVertexBuffers.find(modelName) != modelVertexBuffers.end() && "Model not found");
	return modelVertexBuffers[modelName].GetResource();
}

const D3D12_VERTEX_BUFFER_VIEW* ModelVertexResourceManager::GetVertexBufferView(const std::string& modelName) {
	assert(modelVertexBuffers.find(modelName) != modelVertexBuffers.end() && "Model not found");
	return modelVertexBuffers[modelName].GetVertexBufferView();
}
