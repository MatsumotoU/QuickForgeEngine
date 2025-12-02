#include "SpriteRenderer.h"
#include "Assets/AssetManager.h"
#include "Assets/3DModel/Data/ModelRenderData.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "Graphic/Pipeline/GraphicPipelineManager.h"
#include "Assets/Sprite/Data/SpriteData.h" 
#include "Graphic/TempGraphic.h"
#include <cassert>

void Render::Sprite::DrawSprite(const uint32_t& spriteHandle) {
	AssetManager* assetManager = AssetManager::GetInstance();
	assert(assetManager && "AssetManager is nullptr.");
	const SpriteData& spriteData = assetManager->GetEntityManager()->GetComponent<SpriteData>(spriteHandle);
	if (!spriteData.isDraw) {
		return;
	}

	PipelineStateObject* pso = GraphicPipelineManager::GetInstance()->GetTrianglePso(kBlendModeNormal);

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	assert(dxCommon && "DirectXCommon is nullptr.");
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT);

	commandList->RSSetViewports(1, dxCommon->GetViewPort());
	commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());
	commandList->SetGraphicsRootSignature(pso->GetRootSignature());
	commandList->SetPipelineState(pso->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1,
		assetManager->GetSpriteManager()->GetVertexBuffer(spriteData.vertexBufferHandle)->GetVertexBufferView());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0,
		assetManager->GetMaterialBufferManager()->GetBufferAddress(spriteData.materialBufferHandle));
	commandList->SetGraphicsRootConstantBufferView(1,
		assetManager->GetWpvBufferManager()->GetBufferAddress(spriteData.wvpBufferHandle));
	commandList->SetGraphicsRootDescriptorTable(2,
		assetManager->GetTextureManager()->GetTextureSrvHandleGPU(spriteData.textureHandle));
	commandList->SetGraphicsRootConstantBufferView(3,
		assetManager->GetLightBufferManager()->GetBufferAddress(spriteData.lightBufferHandle));
	commandList->SetGraphicsRootConstantBufferView(4,
		TempGraphic::GetInstance()->GetEchoSphereBuffer()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(5,
		TempGraphic::GetInstance()->GetEchoSphereStructuredBuffer()->GetInstancingSrvHandles().gpuHandle_);
	commandList->DrawInstanced(6, 1, 0, 0);
}
