#include "engine/include/renderer/SpriteRenderer.h"
#include "engine/include/assets/AssetManager.h"
#include "engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include "engine/include/assets/Sprite/Data/SpriteData.h" 
#include <cassert>

void Render::Sprite::DrawSprite(const uint32_t& spriteHandle) {
	AssetManager* assetManager = AssetManager::GetInstance();
	GpuBufferPool* gpuBufferPool = assetManager->GetGpuBufferPool();
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
		gpuBufferPool->GetConstantBufferAddress<Material>(spriteData.materialBufferHandle));
	commandList->SetGraphicsRootConstantBufferView(1,
		gpuBufferPool->GetConstantBufferAddress<TransformationMatrix>(spriteData.wvpBufferHandle));
	commandList->SetGraphicsRootDescriptorTable(2,
		assetManager->GetTextureManager()->GetTextureSrvHandleGPU(spriteData.textureHandle));
	commandList->SetGraphicsRootConstantBufferView(3,
		gpuBufferPool->GetConstantBufferAddress<DirectionalLight>(spriteData.lightBufferHandle));
	commandList->DrawInstanced(6, 1, 0, 0);
}
