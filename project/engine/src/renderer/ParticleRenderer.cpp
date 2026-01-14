/**
 * @file ParticleRenderer.cpp
 * @brief パーティクルレンダリング機能の実装
 */

#include "engine/include/renderer/ParticleRenderer.h"
#include "engine/include/assets/AssetManager.h"

#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"
#include <cassert>

#include "engine/include/assets/Particle/Data/ParticleComponent.h"

/**
 * @brief パーティクルの描画実行
 * @param particleHandle 描画するパーティクル(エンティティ等)のハンドル
 */
void Render::Particle::DrawParticles(const uint32_t& particleHandle) {
	AssetManager* assetManager = AssetManager::GetInstance();
	GpuBufferPool* gpuBufferPool = assetManager->GetGpuBufferPool();
    // TODO: particleHandle の有効性チェックが必要
	ParticleComponent particleComponent = assetManager->GetEntityManager()->GetComponent<ParticleComponent>(particleHandle);

	PipelineStateObject* pso = GraphicPipelineManager::GetInstance()->GetParticlePso(kBlendModeNormal);

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	assert(dxCommon && "DirectXCommon is nullptr.");
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT);

	commandList->RSSetViewports(1, dxCommon->GetViewPort());
	commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());
	commandList->SetGraphicsRootSignature(pso->GetRootSignature());
	commandList->SetPipelineState(pso->GetPipelineState());

	commandList->IASetVertexBuffers(0, 1, assetManager->GetModelVertexResourceManager()->GetVertexBufferView(particleComponent.vartexBufferHandle));
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, gpuBufferPool->GetConstantBufferAddress<Material>(particleComponent.materialHandle));
	commandList->SetGraphicsRootDescriptorTable(1, assetManager->GetParticleGpuDataManager()->GetBufferPtr(particleComponent.particleGpuBufferHandle)->GetInstancingSrvHandles().gpuHandle_);
	commandList->SetGraphicsRootDescriptorTable(2, assetManager->GetTextureManager()->GetTextureSrvHandleGPU(particleComponent.textureHandle));
	commandList->DrawInstanced(static_cast<UINT>(
		assetManager->GetModelVertexResourceManager()->GetVertexBufferCount(particleComponent.vartexBufferHandle)), particleComponent.maxParticleCount, 0, 0);
}
