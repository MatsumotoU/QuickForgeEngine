#include "DrawMesh.h"
#include "Base/EngineCore.h"
#include "Object/Entity/EntityManager.h"
#include "Object/Component/Data/MeshComponent.h"
#include "Object/Component/Data/MaterialComponent.h"
#include "Object/Component/Data/TransformComponent.h"
#include "Base/DirectX/PipelineStateObject.h"

void DrawMesh::Draw(EngineCore* engineCore, EntityManager& entity, Camera& camera) {
	const ComponentStrage<MeshComponent>& strage = entity.GetComponentStrage<MeshComponent>();
	for (const auto& pair : strage) {
		uint32_t entityId = pair.first;
		const MeshComponent& mesh = pair.second;

		ID3D12GraphicsCommandList* commandList = engineCore->GetDirectXCommon()->GetCommandList();
		commandList->RSSetViewports(1, camera.viewport_.GetViewport());
		commandList->RSSetScissorRects(1, camera.scissorrect_.GetScissorRect());
		if (entity.HasComponent<MaterialComponent>(entityId)&& entity.HasComponent<TransformComponent>(entityId)) {
			const MaterialComponent& material = entity.GetComponent<MaterialComponent>(entityId);
			const TransformComponent& transform = entity.GetComponent<TransformComponent>(entityId);
			PipelineStateObject* pso = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
			commandList->SetPipelineState(pso->GetPipelineState());

			commandList->IASetVertexBuffers(0, 1, mesh.vertexBuffers_.GetVertexBufferView());
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			commandList->SetGraphicsRootConstantBufferView(0, material.material_.GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(1,transform.transformationBuffer_.GetGPUVirtualAddress());
			commandList->SetGraphicsRootConstantBufferView(3, material.directionalLight_.GetGPUVirtualAddress());
			commandList->SetGraphicsRootDescriptorTable(2, engineCore->GetTextureManager()->GetTextureSrvHandleGPU(material.textureHandle));

		} else {
			PipelineStateObject* pso = engineCore->GetGraphicsCommon()->GetPrimitivePso(BlendMode::kBlendModeNormal);
			commandList->SetGraphicsRootSignature(pso->GetRootSignature());
			commandList->SetPipelineState(pso->GetPipelineState());
			commandList->IASetVertexBuffers(0, 1, mesh.vertexBuffers_.GetVertexBufferView());
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		}

		// 描画
		commandList->DrawInstanced(static_cast<UINT>(mesh.vertices.size()), 1, 0, 0);
	}
}
