#include "Particle.h"
#include "../Base/EngineCore.h"
#include "../Base/DirectX/DirectXCommon.h"
#include "../Base/DirectX/TextureManager.h"
#include "../Base/DirectX/DepthStencil.h"
#include "../Model/ModelManager.h"

#include "../Camera/Camera.h"

void Particle::Initialize(EngineCore* engineCore, uint32_t totalParticles) {
	totalParticles_ = totalParticles;
	engineCore_ = engineCore;

	wvp_.Initialize(engineCore_->GetDirectXCommon(), totalParticles_);
	material_.Initialize(engineCore_->GetDirectXCommon());
	directionalLight_.Initialize(engineCore_->GetDirectXCommon());
	material_.materialData_->enableLighting = false;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = totalParticles;
	srvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);

	instancingSrvHandleCPU_ = GetCPUDecriptorHandle(
		engineCore_->GetSrvDescriptorHeap()->GetSrvDescriptorHeap(), engineCore_->GetDirectXCommon()->GetDescriptorSizeSRV(), 2);
	instancingSrvHandleGPU_ = GetGPUDecriptorHandle(
		engineCore_->GetSrvDescriptorHeap()->GetSrvDescriptorHeap(), engineCore_->GetDirectXCommon()->GetDescriptorSizeSRV(), 2);
	engineCore_->GetDirectXCommon()->GetDevice()->CreateShaderResourceView(wvp_.GetWVPResource(), &srvDesc, instancingSrvHandleCPU_);

	pso_ = engineCore_->GetGraphicsCommon()->GetParticlePso(kBlendModeNormal);
}

void Particle::LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem) {
	modelData_ = Modelmanager::LoadObjFile(directoryPath, filename, coordinateSystem);
	vertexResource_ = CreateBufferResource(engineCore_->GetDirectXCommon()->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	// テクスチャを読み込む
	modelTextureHandle_ = engineCore_->GetTextureManager()->LoadTexture(modelData_.material.textureFilePath);
}

void Particle::Draw(std::vector<Transform>* transform, Camera* camera) {
	for (uint32_t index = 0; index < transform->size(); index++) {
		Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix((*transform)[index].scale, (*transform)[index].rotate, (*transform)[index].translate);
		Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_PERSPECTIVE);
		wvp_.SetWorldMatrix(worldMatrix, index);
		wvp_.SetWVPMatrix(wvpMatrix, index);
	}
	
	DirectXCommon* dxCommon_ = engineCore_->GetDirectXCommon();
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	commandList->RSSetViewports(1, camera->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU_);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetDirectionalLightResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, engineCore_->GetTextureManager()->GetTextureSrvHandleGPU(modelTextureHandle_));
	commandList->DrawInstanced(static_cast<UINT>(modelData_.vertices.size()), totalParticles_, 0, 0);
}
