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

Billboard::Billboard(EngineCore* engineCore, float width, float hight, uint32_t textureHandle) : BaseGameObject(engineCore) {
	engineCore_ = engineCore;
	dxCommon_ = engineCore->GetDirectXCommon();
	textureManager_ = engineCore->GetTextureManager();
	pso_ = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);
	worldMatrix_ = Matrix4x4::MakeIndentity4x4();
	size_.x = width;
	size_.y = hight;

	// 初期化
	wvp_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	directionalLight_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	material_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());

	// 頂点データの作成
	vertexBuffer_.CreateResource(dxCommon_->GetDevice(), 4);
	vertexBuffer_.GetData()[0].position = { -width * 0.5f, hight * 0.5f, 0.0f, 1.0f };
	vertexBuffer_.GetData()[0].texcoord = { 0.0f, 1.0f };
	vertexBuffer_.GetData()[0].normal = { 0.0f, 0.0f, -1.0f };
	vertexBuffer_.GetData()[1].position = { -width * 0.5f, -hight * 0.5f, 0.0f, 1.0f };
	vertexBuffer_.GetData()[1].texcoord = { 0.0f, 0.0f };
	vertexBuffer_.GetData()[1].normal = { 0.0f, 0.0f, -1.0f };
	vertexBuffer_.GetData()[2].position = { width * 0.5f, hight * 0.5f, 0.0f, 1.0f };
	vertexBuffer_.GetData()[2].texcoord = { 1.0f, 1.0f };
	vertexBuffer_.GetData()[2].normal = { 0.0f, 0.0f, -1.0f };
	vertexBuffer_.GetData()[3].position = { width * 0.5f, -hight * 0.5f, 0.0f, 1.0f };
	vertexBuffer_.GetData()[3].texcoord = { 1.0f, 0.0f };
	vertexBuffer_.GetData()[3].normal = { 0.0f, 0.0f, -1.0f };

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

	modelTextureHandle_ = textureHandle;
}

void Billboard::Init() {
	wvp_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	wvp_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();
	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4();
}

void Billboard::Update() {
	// こっちにワールド行列を移す
}

void Billboard::Draw(Camera* camera) {
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, camera->transform_.rotate, transform_.translate);
	Matrix4x4 wvpMatrix = camera->MakeWorldViewProjectionMatrix(worldMatrix_, CAMERA_VIEW_STATE_PERSPECTIVE);
	wvp_.GetData()->World = worldMatrix_;
	wvp_.GetData()->WVP = wvpMatrix;

	// billboard
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, camera->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, vertexBuffer_.GetVertexBufferView());
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(modelTextureHandle_));
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetGPUVirtualAddress());
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}