#include "Sprite.h"
#include "../Base/DirectX/DirectXCommon.h"
#include "../Base/DirectX/TextureManager.h"
#include "../Math/VerTexData.h"
#include "../Base/DirectX/DepthStencil.h"
#include "../Base/DirectX/PipelineStateObject.h"

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

#include "../Base/DirectX/ImGuiManager.h"

#include "../Camera/Camera.h"
#include "../Base/DirectX/Viewport.h"
#include "../Base/DirectX/ScissorRect.h"

Sprite::Sprite() {
	
}

Sprite::~Sprite() {
}

void Sprite::Initialize(DirectXCommon* dxCommon, TextureManager* textureManager, ImGuiManager* imguiManager, float width, float hight, PipelineStateObject* pso) {
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
	imGuiManager_ = imguiManager;
	pso_ = pso;

	material_.Initialize(dxCommon);
	wvp_.Initialize(dxCommon);
	directionalLight_.Initialize(dxCommon);

	// Spriteを作る
	vertexResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(VertexData) * 4);
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データ作成
	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vertexData_[0].position = { 0.0f,hight,0.0f,1.0f };
	vertexData_[0].texcoord = { 0.0f,1.0f };
	vertexData_[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexData_[1].texcoord = { 0.0f,0.0f };
	vertexData_[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[2].position = { width,hight,0.0f,1.0f };
	vertexData_[2].texcoord = { 1.0f,1.0f };
	vertexData_[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[3].position = { width,0.0f,0.0f,1.0f };
	vertexData_[3].texcoord = { 1.0f,0.0f };
	vertexData_[3].normal = { 0.0f,0.0f,-1.0f };

	// indexBufferの作成
	indexResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(uint32_t) * 6);
	indexBufferView_ = {};
	indexBufferView_.BufferLocation = indexResource_.Get()->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	indexData_ = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	indexData_[0] = 0;
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 1;
	indexData_[4] = 3;
	indexData_[5] = 2;
}

void Sprite::DrawSprite(const Transform& transform, int32_t textureHandle, Camera* camera) {
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_ORTHOGRAPHIC);
	wvp_.SetWorldMatrix(worldMatrix);
	wvp_.SetWVPMatrix(wvpMatrix);

	// sprite
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, camera->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetWVPResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureHandle));
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetDirectionalLightResource()->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::DrawSprite(const Transform& transform, const Transform& uvTransform, int32_t textureHandle, Camera* camera) {
	// uv
	Matrix4x4 uvTransformMatrix = Matrix4x4::MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeTranslateMatrix(uvTransform.translate));
	material_.SetUvTransformMatrix(uvTransformMatrix);

	// wvp
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_ORTHOGRAPHIC);
	wvp_.SetWorldMatrix(worldMatrix);
	wvp_.SetWVPMatrix(wvpMatrix);

	// sprite
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	/*D3D12_CPU_DESCRIPTOR_HANDLE dsvHandl = pso_->GetDepthStencil()->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, dxCommon_->GetRtvHandles(), false, &dsvHandl);
	commandList->ClearDepthStencilView(dsvHandl, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);*/

	commandList->RSSetViewports(1, camera->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetWVPResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureHandle));
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetDirectionalLightResource()->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::DrawSprite(const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) {
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetWVPResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, gpuHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetDirectionalLightResource()->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void Sprite::DrawSprite(const Transform& transform, const Transform& uvTransform, const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle, Camera* camera) {
	// uv
	Matrix4x4 uvTransformMatrix = Matrix4x4::MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Matrix4x4::Multiply(uvTransformMatrix, Matrix4x4::MakeTranslateMatrix(uvTransform.translate));
	material_.SetUvTransformMatrix(uvTransformMatrix);

	// wvp
	Matrix4x4 worldMatrix = Matrix4x4::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix, CAMERA_VIEW_STATE_ORTHOGRAPHIC);
	wvp_.SetWorldMatrix(worldMatrix);
	wvp_.SetWVPMatrix(wvpMatrix);

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetWVPResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, gpuHandle);
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetDirectionalLightResource()->GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
