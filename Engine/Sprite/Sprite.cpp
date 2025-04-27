#include "Sprite.h"
#include "../Base/DirectX/DirectXCommon.h"
#include "../Base/DirectX/TextureManager.h"
#include "../Math/VerTexData.h"
#include "../Base/DirectX/PipelineStateObject.h"

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionalLightResource.h"

#include "../Base/DirectX/ImGuiManager.h"

Sprite::Sprite(DirectXCommon* dxCommon, TextureManager* textureManager, ImGuiManager* imguiManager, float width, float hight, PipelineStateObject* pso) {
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

Sprite::~Sprite() {
}

void Sprite::DrawSprite(int32_t textureHandle,ViewPort* viewport,ScissorRect* scissor) {
	// sprite
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, viewport->GetViewport());
	commandList->RSSetScissorRects(1, scissor->GetScissorRect());
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