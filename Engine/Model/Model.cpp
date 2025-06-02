#include "Model.h"
#include "ModelManager.h"
#include "../Base/EngineCore.h"
#include "../Base/DirectX/DirectXCommon.h"
#include "../Base/DirectX/TextureManager.h"
#include "../Base/DirectX/DepthStencil.h"

#include "../Camera/Camera.h"

void Model::Initialize(EngineCore* engineCore) {
	engineCore_ = engineCore;

	dxCommon_ = engineCore->GetDirectXCommon();
	textureManager_ = engineCore->GetTextureManager();
	pso_ = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);

	material_.Initialize(dxCommon_);
	wvp_.Initialize(dxCommon_,1);
	directionalLight_.Initialize(dxCommon_);
}

void Model::LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem) {
	modelData_ = Modelmanager::LoadObjFile(directoryPath, filename, coordinateSystem);
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData_.vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	std::memcpy(vertexData_, modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	// テクスチャを読み込む
	modelTextureHandle_ = textureManager_->LoadTexture(modelData_.material.textureFilePath);
}

void Model::Draw(const Transform& transform, Camera* camera) {
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_PERSPECTIVE);
	wvp_.SetWorldMatrix(worldMatrix,0);
	wvp_.SetWVPMatrix(wvpMatrix,0);

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	commandList->RSSetViewports(1, camera->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetWVPResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetDirectionalLightResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(modelTextureHandle_));
	commandList->DrawInstanced(static_cast<UINT>(modelData_.vertices.size()), 1, 0, 0);
}

void Model::SetBlendmode(BlendMode mode) {
	pso_ = engineCore_->GetGraphicsCommon()->GetTrianglePso(mode);
}
