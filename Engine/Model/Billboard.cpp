#include "Billboard.h"
#include "Base/EngineCore.h"
#include "Base/DirectX/DirectXCommon.h"
#include "Base/DirectX/TextureManager.h"
#include "Math/VerTexData.h"
#include "Base/DirectX/DepthStencil.h"
#include "Base/DirectX/PipelineStateObject.h"

#include "Base/DirectX/MaterialResource.h"
#include "Base/DirectX/WVPResource.h"
#include "Base/DirectX/DirectionalLightResource.h"

#include "Base/DirectX/ImGuiManager.h"

#include "Camera/Camera.h"
#include "Base/DirectX/Viewport.h"
#include "Base/DirectX/ScissorRect.h"

void Billboard::Initialize(EngineCore* engineCore, float width, float hight) {
	engineCore_ = engineCore;
	dxCommon_ = engineCore->GetDirectXCommon();
	textureManager_ = engineCore_->GetTextureManager();
	pso_ = engineCore_->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);

	material_.Initialize(dxCommon_);
	wvp_.Initialize(dxCommon_, 1);
	directionalLight_.Initialize(dxCommon_);

	// Spriteを作る
	vertexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * 4);
	vertexBufferView_ = {};
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点データ作成
	vertexData_ = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	float halfHight = hight * 0.5f;
	float halfWidth = width *0.5f;

	vertexData_[0].position = { -halfWidth,halfHight,0.0f,1.0f };
	vertexData_[0].texcoord = { 0.0f,1.0f };
	vertexData_[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[1].position = { -halfWidth,-halfHight,0.0f,1.0f };
	vertexData_[1].texcoord = { 0.0f,0.0f };
	vertexData_[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[2].position = { halfWidth,halfHight,0.0f,1.0f };
	vertexData_[2].texcoord = { 1.0f,1.0f };
	vertexData_[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData_[3].position = { halfWidth,-halfHight,0.0f,1.0f };
	vertexData_[3].texcoord = { 1.0f,0.0f };
	vertexData_[3].normal = { 0.0f,0.0f,-1.0f };

	// indexBufferの作成
	indexResource_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * 6);
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

	transform_ = {};
	worldMatrix_ = Matrix4x4::MakeIndentity4x4();
}

void Billboard::Update(const Vector3& cameraRotate) {
	transform_.rotate = cameraRotate;
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

void Billboard::Draw(int32_t textureHandle, Camera* camera) {
	ImGui::DragFloat3("translate", &transform_.translate.x);
	ImGui::DragFloat3("rotate", &transform_.rotate.x);
	ImGui::DragFloat3("scale", &transform_.scale.x);

	Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix_, CAMERA_VIEW_STATE_PERSPECTIVE);
	wvp_.SetWorldMatrix(worldMatrix_, 0);
	wvp_.SetWVPMatrix(wvpMatrix, 0);

	// billboard
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