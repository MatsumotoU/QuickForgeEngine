#include "D3D12GraphicFrameWork.h"

#include "graphics/D3D12GraphicEngine.h"
#include "graphics/dx12/pipeline/GraphicPipelineManager.h"
#include "graphics/dx12/pipeline/RaytracingPipelineManager.h"
#include "graphics/dx12/pipeline/ComputePipelineManager.h"
#include "graphics/dx12/vram/DirectXResourceAllocator.h"
#include "graphics/dx12/vram/resources/DirectXResourceContainer.h"
#include "graphics/dx12/TextureLoader.h"
#include "graphics/dx12/DirectXDevice.h"
#include "graphics/dx12/command/DirectXCommandManager.h"
#include "graphics/dx12/RenderPass.h"
#include "graphics/dx12/pipeline/rtpso/RaytracingAccelerationStructure.h"
#include "graphics/dx12/Fence.h"

#include "graphics/dx12/pipeline/rtpso/TLAS.h"

#include "core/math/transform/Transform.h"
#include "assetfactory/model/ModelData.h"
#include "../../resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> QFE::FRAMEWORK::CreateGraphicEngine(HWND hwnd) {
	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::GRAPHIC::D3D12GraphicEngine> graphicEngine =
		std::make_unique<QFE::GRAPHIC::D3D12GraphicEngine>(hwnd);
	graphicEngine->Initialize();
	return graphicEngine;
}

bool QFE::FRAMEWORK::PreDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->PreDraw();
	return true;
}

bool QFE::FRAMEWORK::PostDrawGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->PostDraw();
	return true;
}

bool QFE::FRAMEWORK::ShutdownGraphicEngine(QFE::GRAPHIC::D3D12GraphicEngine* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->Shutdown();
	return true;
}

void QFE::FRAMEWORK::BuildGlobalMeshBuffers(
	const std::unordered_map<std::string, QFE::ASSET::ModelData>& modelDataMap,
	const std::map<std::string, uint32_t>& textureGpuIndexMap,
	std::vector<float>& outGlobalUVs,
	std::vector<uint32_t>& outGlobalTriIndices,
	std::unordered_map<std::string, InstanceMetaCPU>& outModelMeta) {
	outGlobalUVs.clear();
	outGlobalTriIndices.clear();
	outModelMeta.clear();

	for (const auto& [modelName, model] : modelDataMap) {
		if (model.meshes.empty()) continue;
		const QFE::ASSET::MeshData& mesh = model.meshes[0];
		const uint32_t vertexBase = static_cast<uint32_t>(outGlobalUVs.size() / 2);
		const uint32_t primitiveBase = static_cast<uint32_t>(outGlobalTriIndices.size() / 3);
		const auto& vertices = mesh.vertices.GetInternalVector();
		for (const auto& vertex : vertices) {
			outGlobalUVs.push_back(vertex.texcoord.x);
			outGlobalUVs.push_back(vertex.texcoord.y);
		}
		const auto& indices = mesh.indices.GetInternalVector();
		outGlobalTriIndices.insert(outGlobalTriIndices.end(), indices.begin(), indices.end());

		InstanceMetaCPU meta{};
		meta.materialIndex = 1u;
		if (const auto texture = textureGpuIndexMap.find(mesh.material.textureName);
			texture != textureGpuIndexMap.end()) {
			meta.materialIndex = texture->second;
		}
		meta.vertexBase = vertexBase;
		meta.vertexCount = static_cast<uint32_t>(vertices.size());
		meta.primitiveBase = primitiveBase;
		outModelMeta[modelName] = meta;
	}
}
