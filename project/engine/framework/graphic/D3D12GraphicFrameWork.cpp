#include "D3D12GraphicFrameWork.h"

#include "GraphicContext.h"
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

std::unique_ptr<QFE::FRAMEWORK::GraphicContext> QFE::FRAMEWORK::CreateGraphicEngine(HWND hwnd) {
	// ウィンドウのハンドルを取得してグラフィックエンジンを初期化
	std::unique_ptr<QFE::FRAMEWORK::GraphicContext> graphicEngine =
		std::make_unique<QFE::FRAMEWORK::GraphicContext>(hwnd);
	graphicEngine->Initialize();
	return graphicEngine;
}

bool QFE::FRAMEWORK::PreDrawGraphicEngine(QFE::FRAMEWORK::GraphicContext* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->PreDraw();
	return true;
}

bool QFE::FRAMEWORK::PostDrawGraphicEngine(QFE::FRAMEWORK::GraphicContext* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->PostDraw();
	return true;
}

bool QFE::FRAMEWORK::ShutdownGraphicEngine(QFE::FRAMEWORK::GraphicContext* graphicEngine) {
	if(graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}
	graphicEngine->Shutdown();
	return true;
}

bool QFE::FRAMEWORK::CreateEditorSceneTexture(
	GraphicContext* graphicEngine,
	QFE::GRAPHIC::RenderTargetHandle& outRenderTargetHandle,
	QFE::GRAPHIC::DirectXResourceHandle& outResourceHandle,
	uintptr_t& outGuiTextureId,
	uint32_t width,
	uint32_t height) {
	if (graphicEngine == nullptr) {
		QFE_LOG("graphicEngine is null");
		return false;
	}

	if (!CreateOffScreenRenderTarget(
		graphicEngine,
		outRenderTargetHandle,
		width,
		height,
		DXGI_FORMAT_R8G8B8A8_UNORM)) {
		return false;
	}

	if (!GetRenderResourceHandle(
		graphicEngine,
		outRenderTargetHandle,
		outResourceHandle)) {
		return false;
	}

	if (!TransitionResourceToState(
		graphicEngine,
		outResourceHandle,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)) {
		return false;
	}

	const auto gpuHandle = graphicEngine->GetDirectXResourceContainer()->GetDescriptorHandleGPU(
		outResourceHandle,
		D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
	outGuiTextureId = static_cast<uintptr_t>(gpuHandle.ptr);
	return true;
}

void QFE::FRAMEWORK::BuildGlobalMeshBuffers(
	const std::unordered_map<std::string, QFE::ASSET::ModelData>& modelDataMap,
	const std::map<std::string, uint32_t>& textureGpuIndexMap,
	std::vector<RaytracingVertexAttribute>& outGlobalVertexAttributes,
	std::vector<uint32_t>& outGlobalTriIndices,
	std::unordered_map<std::string, InstanceMetaCPU>& outModelMeta) {
	outGlobalVertexAttributes.clear();
	outGlobalTriIndices.clear();
	outModelMeta.clear();

	for (const auto& [modelName, model] : modelDataMap) {
		if (model.meshes.empty()) continue;
		const QFE::ASSET::MeshData& mesh = model.meshes[0];
		const uint32_t vertexBase = static_cast<uint32_t>(outGlobalVertexAttributes.size());
		const uint32_t primitiveBase = static_cast<uint32_t>(outGlobalTriIndices.size() / 3);
		const auto& vertices = mesh.vertices.GetInternalVector();
		for (const auto& vertex : vertices) {
			RaytracingVertexAttribute attribute{};
			attribute.texcoord = vertex.texcoord;
			attribute.normal = vertex.normal;
			outGlobalVertexAttributes.push_back(attribute);
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
