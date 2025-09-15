#include "pch.h"
#include "ModelRenderer.h"
#include "Assets/AssetManager.h"
#include "Assets/3DModel/Data/ModelRenderData.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "Graphic/Pipeline/GraphicPipelineManager.h"
#include <cassert>

void Render::Model::DrawModel(const uint32_t& modelHandle) {
	AssetManager* assetManager = AssetManager::GetInstance();
	assert(assetManager && "AssetManager is nullptr.");
	const ModelRenderData* modelDataPtr = assetManager->GetModelRenderData(modelHandle);

	PipelineStateObject* pso = GraphicPipelineManager::GetInstance()->GetTrianglePso(kBlendModeNormal);

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	assert(dxCommon && "DirectXCommon is nullptr.");
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT);

	commandList->RSSetViewports(1, dxCommon->GetViewPort());
	commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());
	commandList->SetGraphicsRootSignature(pso->GetRootSignature());
	commandList->SetPipelineState(pso->GetPipelineState());

	for (auto& handle : modelDataPtr->meshRenderDataHandles) {
		commandList->IASetVertexBuffers(0, 1,
			assetManager->GetModelVertexResourceManager()->GetVertexBufferView(handle.vertexBufferHandle));
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetGraphicsRootConstantBufferView(0, 
			assetManager->GetMaterialBufferManager()->GetBufferAddress(handle.materialHandle));
		commandList->SetGraphicsRootConstantBufferView(1, 
			assetManager->GetWpvBufferManager()->GetBufferAddress(handle.wpvBufferHandle));
		commandList->SetGraphicsRootConstantBufferView(3, 
			assetManager->GetLightBufferManager()->GetBufferAddress(handle.lightBufferHandle));
		commandList->SetGraphicsRootDescriptorTable(2, 
			assetManager->GetTextureManager()->GetTextureSrvHandleGPU(handle.textureHandle));
		commandList->DrawInstanced(static_cast<UINT>(modelDataPtr->meshRenderDataHandles.size()), 1, 0, 0);
	}
}
