#include "engine/include/renderer/SkyboxRenderer.h"

#include "engine/include/assets/AssetManager.h"
#include "engine/include/assets/3DModel/Data/ModelRenderData.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/graphic/Pipeline/GraphicPipelineManager.h"

#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

#include "engine/include/core/EngineDefines.h"

void QFE::Render::Skybox::DrawSkybox(const SkyboxComponent& skyboxComponent)
{
	// コンポーネントに必要なデータがセットされているかのチェック
	if (skyboxComponent.textureHandle == UINT32_MAX ||
		skyboxComponent.vertexBufferHandle == UINT32_MAX ||
		skyboxComponent.materialBufferHandle == UINT32_MAX) {
		QFE_LOG("Error: SkyboxComponent has invalid handles. Cannot draw skybox.");
		return;
	}
	
	AssetManager* assetManager = AssetManager::GetInstance();
	assert(assetManager && "AssetManager is nullptr.");

	PipelineStateObject* pso = GraphicPipelineManager::GetInstance()->GetSkyBoxPso();

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	assert(dxCommon && "DirectXCommon is nullptr.");
	ID3D12GraphicsCommandList* commandList = dxCommon->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT);

	commandList->RSSetViewports(1, dxCommon->GetViewPort());
	commandList->RSSetScissorRects(1, dxCommon->GetScissorRect());
	commandList->SetGraphicsRootSignature(pso->GetRootSignature());
	commandList->SetPipelineState(pso->GetPipelineState());

	commandList->IASetVertexBuffers(0, 1,
		assetManager->GetModelVertexResourceManager()->GetVertexBufferView(skyboxComponent.vertexBufferHandle));
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0,
		assetManager->GetGpuBufferPool()->GetConstantBufferAddress<TransformationMatrix>(skyboxComponent.wvpBufferHandle));
	commandList->SetGraphicsRootConstantBufferView(1,
		assetManager->GetGpuBufferPool()->GetConstantBufferAddress<Material>(skyboxComponent.materialBufferHandle));
	commandList->SetGraphicsRootDescriptorTable(2,
		assetManager->GetTextureManager()->GetTextureSrvHandleGPU(skyboxComponent.textureHandle));

	commandList->DrawInstanced(36, 1, 0, 0);
}
