#include "Particle.h"
#include "../Base/EngineCore.h"
#include "../Base/DirectX/DirectXCommon.h"
#include "../Base/DirectX/TextureManager.h"
#include "../Base/DirectX/DepthStencil.h"
#include "../Model/ModelManager.h"

#include "../Camera/Camera.h"

#include "Particle/ParticleForGPU.h"
#include "Model/AssimpModelLoader.h"

// インスタンスの総数初期化
uint32_t Particle::instanceCount_ = 0;

Particle::Particle() {
	instanceCount_ ++;
}

void Particle::Initialize(EngineCore* engineCore, uint32_t totalParticles) {
	totalParticles_ = totalParticles;
	engineCore_ = engineCore;

	wvp_.Initialize(engineCore_->GetDirectXCommon(), totalParticles_,true);
	material_.Initialize(engineCore_->GetDirectXCommon());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = totalParticles;
	srvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	instancingSrvHandles_ = engineCore_->GetSrvDescriptorHeap()->AssignEmptyArrayHandles();
	engineCore_->GetSrvDescriptorHeap()->AssignHeap(wvp_.GetWVPResource(), srvDesc, instancingSrvHandles_.cpuHandle_);

	pso_ = engineCore_->GetGraphicsCommon()->GetParticlePso(kBlendModeNormal);
}

void Particle::LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem) {
	modelData_ = AssimpModelLoader::LoadModelData(directoryPath, filename, coordinateSystem);

	// 単一メッシュ前提
	if (modelData_.meshes.empty()) {
		// エラー処理
		return;
	}
	const auto& mesh = modelData_.meshes[0];

	vertexResource_ = CreateBufferResource(
		engineCore_->GetDirectXCommon()->GetDevice(),
		sizeof(VertexData) * mesh.vertices.size()
	);
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * mesh.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, mesh.vertices.data(), sizeof(VertexData) * mesh.vertices.size());

	// テクスチャを読み込む
	modelTextureHandle_ = engineCore_->GetTextureManager()->LoadTexture(mesh.material.textureFilePath);
}

void Particle::Draw(std::vector<Transform>* transform, std::vector<Vector4>* color, Camera* camera) {
	for (uint32_t index = 0; index < transform->size(); index++) {
		Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix((*transform)[index].scale, (*transform)[index].rotate, (*transform)[index].translate);
		Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_PERSPECTIVE);
		wvp_.SetWorldMatrix(worldMatrix, index);
		wvp_.SetWVPMatrix(wvpMatrix, index);
	}

	// 単一メッシュ前提
	if (modelData_.meshes.empty()) {
		return;
	}
	const auto& mesh = modelData_.meshes[0];

	for (uint32_t index = 0; index < color->size(); index++) {
		wvp_.particleData_[index].color = (*color)[index];
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
	commandList->SetGraphicsRootDescriptorTable(1, instancingSrvHandles_.gpuHandle_);
	commandList->SetGraphicsRootDescriptorTable(2, engineCore_->GetTextureManager()->GetTextureSrvHandleGPU(modelTextureHandle_));
	commandList->DrawInstanced(static_cast<UINT>(mesh.vertices.size()), totalParticles_, 0, 0);
}
