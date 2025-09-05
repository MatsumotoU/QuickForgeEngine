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

Billboard::Billboard(EngineCore* engineCore, Camera* camera, float width, float hight, uint32_t textureHandle) : BaseGameObject(engineCore, camera) {
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

	type_ = "Billboard"; // オブジェクトのタイプを設定
}

void Billboard::Init() {
	wvp_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	wvp_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();
	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4();

	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4(); // UV変換行列を初期化
	material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	material_.GetData()->enableLighting = true; // ライティングを有効にする
	directionalLight_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	directionalLight_.GetData()->direction = Vector3(0.0f, -1.0f, 0.0f); // 下方向
	directionalLight_.GetData()->intensity = 1.0f; // 輝度
}

void Billboard::Update() {
	// こっちにワールド行列を移す
}

void Billboard::Draw() {
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, camera_->transform_.rotate, transform_.translate);
	Matrix4x4 wvpMatrix = camera_->MakeWorldViewProjectionMatrix(worldMatrix_, ViewState::CAMERA_VIEW_STATE_PERSPECTIVE);
	wvp_.GetData()->World = worldMatrix_;
	wvp_.GetData()->WVP = wvpMatrix;

	// billboard
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, camera_->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera_->scissorrect_.GetScissorRect());
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

nlohmann::json Billboard::Serialize() const {
	nlohmann::json j;
	j["type"] = type_;
	j["name"] = name_;
	j["position"] = { transform_.translate.x, transform_.translate.y, transform_.translate.z };
	j["rotation"] = { transform_.rotate.x, transform_.rotate.y, transform_.rotate.z };
	j["scale"] = { transform_.scale.x, transform_.scale.y, transform_.scale.z };
	j["size"] = { size_.x, size_.y };
	j["modelTextureHandle"] = modelTextureHandle_;
	// コライダー情報を保存
	j["radius"] = GetRadius();
	j["mask"] = GetMask();
	return j;
}

std::unique_ptr<Billboard> Billboard::Deserialize(const nlohmann::json& j, EngineCore* engineCore, Camera* camera) {
	std::unique_ptr<Billboard> billboard = std::make_unique<Billboard>(engineCore, camera, 1.0f, 1.0f, 0);
	billboard->Init();
	// 名前復元
	if (j.contains("name")) billboard->name_ = j["name"].get<std::string>();
	// トランスフォーム復元
	if (j.contains("position")) {
		billboard->transform_.translate.x = j["position"][0].get<float>();
		billboard->transform_.translate.y = j["position"][1].get<float>();
		billboard->transform_.translate.z = j["position"][2].get<float>();
	}
	if (j.contains("rotation")) {
		billboard->transform_.rotate.x = j["rotation"][0].get<float>();
		billboard->transform_.rotate.y = j["rotation"][1].get<float>();
		billboard->transform_.rotate.z = j["rotation"][2].get<float>();
	}
	if (j.contains("scale")) {
		billboard->transform_.scale.x = j["scale"][0].get<float>();
		billboard->transform_.scale.y = j["scale"][1].get<float>();
		billboard->transform_.scale.z = j["scale"][2].get<float>();
	}
	if (j.contains("size")) {
		billboard->size_.x = j["size"][0].get<float>();
		billboard->size_.y = j["size"][1].get<float>();
	}
	if (j.contains("modelTextureHandle")) {
		billboard->modelTextureHandle_ = j["modelTextureHandle"].get<int>();
	}
	// スクリプト名の復元
	if (j.contains("scriptFileName")) {
		billboard->SetScriptName(j["scriptFileName"].get<std::string>());
	}
	// コライダー情報の復元
	if (j.contains("radius")) {
		billboard->SetRadius(j["radius"].get<float>());
	}
	if (j.contains("mask")) {
		billboard->SetMask(j["mask"].get<uint32_t>());
	}
	return billboard;
}

#ifdef _DEBUG
void Billboard::DrawImGui() {
	BaseGameObject::DrawImGui();
}
#endif // _DEBUG