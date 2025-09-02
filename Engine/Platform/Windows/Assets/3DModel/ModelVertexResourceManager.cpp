#include "ModelVertexResourceManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "Loader/AssimpModelLoader.h"
#include <cassert>

void ModelVertexResourceManager::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	assert(dxCommon_);
}

void ModelVertexResourceManager::AssignModelVertexBuffer(const std::string& modelName, const ModelData& modelData) {
	// すでに登録されている場合はスキップ
	if (modelVertexBuffers.find(modelName) != modelVertexBuffers.end()) {
		return;
	}

	// 頂点バッファを作成
	if (modelData.meshes.empty()) {
		assert(false && "ModelData has no meshes");
		return;
	}

	int meshIndex = 0;
	for (const auto& mesh : modelData.meshes) {
		VertexBuffer<VertexData> vertexBuffer;
		std::string meshName = modelName + "_mesh" + std::to_string(meshIndex++);
		modelVertexBuffers[meshName].CreateResource()
	}
}

void ModelVertexResourceManager::LoadModelResources(const std::string& modelResourceDirectory, const std::string& imageResourceDirectory, const std::string& filename) {
	ModelData tempModelData;
	AssimpModelLoader::LoadModelData(modelResourceDirectory, imageResourceDirectory, filename, tempModelData);
	CreateModelVertexBuffer(dxCommon_->GetDevice(), filename);
}

void ModelVertexResourceManager::CreateModelVertexBuffer() {
}
