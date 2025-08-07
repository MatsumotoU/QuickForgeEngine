#include "ModelMeshCreater.h"
#include <cassert>
#include "Base/EngineCore.h"

void ModelMeshCreater::CreateResourcesFromModelData(
	EngineCore* engineCore, const std::vector<VertexData>& vertices, MeshComponent& mashComponent) {

	assert(engineCore);
	if (vertices.empty()) {
		assert(false && "Vertices data is empty. Cannot create resources.");
		return;
	}
	// DirectXリソースを初期化
	mashComponent.vertexBuffers_.CreateResource(engineCore->GetDirectXCommon()->GetDevice(), static_cast<uint32_t>(vertices.size()));
	std::memcpy(mashComponent.vertexBuffers_.GetData(), vertices.data(), sizeof(VertexData) * vertices.size());
	mashComponent.vertices = vertices; // 頂点データを保存
}
