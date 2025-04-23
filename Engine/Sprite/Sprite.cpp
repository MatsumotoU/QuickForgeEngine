#include "Sprite.h"
#include "../Base/DirectX/DirectXCommon.h"
#include "../Base/DirectX/TextureManager.h"
#include "../Math/VerTexData.h"

#include "../Base/DirectX/MaterialResource.h"
#include "../Base/DirectX/WVPResource.h"
#include "../Base/DirectX/DirectionnalLightResource.h"

#include "../Base/DirectX/ImGuiManager.h"

Sprite::Sprite(DirectXCommon* dxCommon, TextureManager* textureManager, ImGuiManager* imguiManager, float width, float hight) {
	dxCommon_ = dxCommon;
	textureManager_ = textureManager;
	imGuiManager_ = imguiManager;

	material_ = new MaterialResource(dxCommon);
	wvp_ = new WVPResource(dxCommon);
	directionalLight_ = new DirectionnalLightResource(dxCommon);

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
	delete material_;
	delete wvp_;
	delete directionalLight_;
	material_ = nullptr;
	wvp_ = nullptr;
	directionalLight_ = nullptr;
}

void Sprite::LoadTexture(const std::string& filePath) {
	// テクスチャを読み込む
	DirectX::ScratchImage mipImages = textureManager_->LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	textureResource_ = textureManager_->CreateTextureResource(metadata);
	textureManager_->CreateShaderResourceView(metadata, imGuiManager_->GetSrvDescriptorHeap(), textureResource_, 1);
}

void Sprite::DrawSprite() {
	// sprite
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->SetGraphicsRootConstantBufferView(0, material_->GetMaterial()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_->GetWVPResource()->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(0));
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}
