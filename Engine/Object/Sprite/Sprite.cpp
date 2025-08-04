#include "Sprite.h"
#include "Base/EngineCore.h"
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

Sprite::Sprite(EngineCore* engineCore, Camera* camera, float width, float hight) : BaseGameObject(engineCore, camera) {
	textureHandle_ = 0;

	engineCore_ = engineCore;
	dxCommon_ = engineCore->GetDirectXCommon();
	textureManager_ = engineCore_->GetTextureManager();
	pso_ = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);

	// 初期化
	wvp_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	directionalLight_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	material_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());

	// ワールド行列を初期化
	worldMatrix_ = Matrix4x4::MakeIndentity4x4();
	// スプライトのサイズを設定
	width_ = width;
	hight_ = hight;
	// 名前を設定
	name_ = "Sprite"; // スプライトの名前を設定
	type_ = "Sprite"; // オブジェクトのタイプを設定
	attachedScriptName.clear();
	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4(); // UV変換行列を初期化
	material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	material_.GetData()->enableLighting = false; // ライティングを有効にする
	directionalLight_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	directionalLight_.GetData()->direction = Vector3(0.0f, -1.0f, 0.0f); // 下方向
	directionalLight_.GetData()->intensity = 1.0f; // 輝度

	// Spriteを作る
	vertexBuffer_.CreateResource(dxCommon_->GetDevice(), 4);

	// 頂点データ作成
	vertexBuffer_.GetData()[0].position = { 0.0f,hight,0.0f,1.0f };
	vertexBuffer_.GetData()[0].texcoord = { 0.0f,1.0f };
	vertexBuffer_.GetData()[0].normal = { 0.0f,0.0f,-1.0f };
	vertexBuffer_.GetData()[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexBuffer_.GetData()[1].texcoord = { 0.0f,0.0f };
	vertexBuffer_.GetData()[1].normal = { 0.0f,0.0f,-1.0f };
	vertexBuffer_.GetData()[2].position = { width,hight,0.0f,1.0f };
	vertexBuffer_.GetData()[2].texcoord = { 1.0f,1.0f };
	vertexBuffer_.GetData()[2].normal = { 0.0f,0.0f,-1.0f };
	vertexBuffer_.GetData()[3].position = { width,0.0f,0.0f,1.0f };
	vertexBuffer_.GetData()[3].texcoord = { 1.0f,0.0f };
	vertexBuffer_.GetData()[3].normal = { 0.0f,0.0f,-1.0f };

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
}

void Sprite::Init() {
	wvp_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	wvp_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();
	directionalLight_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	directionalLight_.GetData()->direction = Vector3(0.0f, -1.0f, 0.0f); // 下方向
	directionalLight_.GetData()->intensity = 1.0f; // 輝度
	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4(); // UV変換行列を初期化
	material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	material_.GetData()->enableLighting = true; // ライティングを有効にする
}

void Sprite::Update() {
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	wvp_.GetData()->World = worldMatrix_;
}

void Sprite::Draw() {
	Matrix4x4 viewMatrix = Matrix4x4::MakeIndentity4x4();
	Matrix4x4 projectionMatrix = Matrix4x4::MakeOrthographicMatrix(0.0f, 0.0f,
		static_cast<float>(engineCore_->GetWinApp()->kWindowWidth),
		static_cast<float>(engineCore_->GetWinApp()->kWindowHeight), 0.0f, 100.0f);
	Matrix4x4 wvpMatrix = Matrix4x4::Multiply(worldMatrix_, Matrix4x4::Multiply(viewMatrix, projectionMatrix));

	wvp_.GetData()->WVP = wvpMatrix;
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();
	commandList->RSSetViewports(1, camera_->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera_->scissorrect_.GetScissorRect());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(textureHandle_));
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetGPUVirtualAddress());
	commandList->IASetVertexBuffers(0, 1, vertexBuffer_.GetVertexBufferView());
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

nlohmann::json Sprite::Serialize() const {
	nlohmann::json j;
	j["type"] = type_;
	j["name"] = name_;
	j["position"] = { transform_.translate.x, transform_.translate.y, transform_.translate.z };
	j["rotation"] = { transform_.rotate.x, transform_.rotate.y, transform_.rotate.z };
	j["scale"] = { transform_.scale.x, transform_.scale.y, transform_.scale.z };
	j["width"] = width_;
	j["hight"] = hight_;
	j["textureHandle"] = textureHandle_;
	j["scriptFileName"] = attachedScriptName;
	// コライダー情報を保存
	j["radius"] = GetRadius();
	j["mask"] = GetMask();
	return j;
}

std::unique_ptr<Sprite> Sprite::Deserialize(const nlohmann::json& j, EngineCore* engineCore, Camera* camera) {
	// 先にwidthとhightを取得しておく
	float width = -1.0f;
	float hight = -1.0f;
	if (j.contains("width")) {
		width = j["width"].get<float>();
	}
	if (j.contains("hight")) {
		hight = j["hight"].get<float>();
	}
	assert(width > 0.0f && hight > 0.0f && "Sprite width and height must be greater than 0");

	// スプライトのインスタンスを作成
	auto sprite = std::make_unique<Sprite>(engineCore, camera, width, hight);
	sprite->Init();
	// 名前復元
	if (j.contains("name")) sprite->name_ = j["name"].get<std::string>();
	// トランスフォーム復元
	if (j.contains("position")) {
		sprite->transform_.translate.x = j["position"][0].get<float>();
		sprite->transform_.translate.y = j["position"][1].get<float>();
		sprite->transform_.translate.z = j["position"][2].get<float>();
	}
	if (j.contains("rotation")) {
		sprite->transform_.rotate.x = j["rotation"][0].get<float>();
		sprite->transform_.rotate.y = j["rotation"][1].get<float>();
		sprite->transform_.rotate.z = j["rotation"][2].get<float>();
	}
	if (j.contains("scale")) {
		sprite->transform_.scale.x = j["scale"][0].get<float>();
		sprite->transform_.scale.y = j["scale"][1].get<float>();
		sprite->transform_.scale.z = j["scale"][2].get<float>();
	}
	if (j.contains("textureHandle")) {
		sprite->textureHandle_ = j["textureHandle"].get<uint32_t>();
	}
	if (j.contains("scriptFileName")) {
		sprite->SetScriptName(j["scriptFileName"].get<std::string>());
	}
	// コライダー情報の復元
	if (j.contains("radius")) {
		sprite->SetRadius(j["radius"].get<float>());
	}
	if (j.contains("mask")) {
		sprite->SetMask(j["mask"].get<uint32_t>());
	}
	return sprite;
}

#ifdef _DEBUG
void Sprite::DrawGizmo(const ImGuizmo::OPERATION& op, const ImGuizmo::MODE& mode, const ImVec2& imageScreenPos, const ImVec2& imageSize) {
	ImGuizmo::SetOrthographic(true);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(imageScreenPos.x, imageScreenPos.y, imageSize.x, imageSize.y);

	// 行列をfloat配列に変換
	Matrix4x4 world = worldMatrix_;
	float worldMatrix[16];
	std::memcpy(worldMatrix, &world, sizeof(float) * 16);

	// ビュー行列（単位行列）
	Matrix4x4 view = Matrix4x4::MakeIndentity4x4();
	float viewMatrix[16];
	std::memcpy(viewMatrix, &view, sizeof(float) * 16);

	// 射影行列（ウィンドウ座標系に合わせる）
	float windowWidth = static_cast<float>(engineCore_->GetWinApp()->kWindowWidth);
	float windowHeight = static_cast<float>(engineCore_->GetWinApp()->kWindowHeight);
	Matrix4x4 proj = Matrix4x4::MakeOrthographicMatrix(
		0.0f, 0.0f,
		windowWidth, windowHeight,
		0.0f, 100.0f
	);
	float projMatrix[16];
	std::memcpy(projMatrix, &proj, sizeof(float) * 16);

	ImGuizmo::Manipulate(viewMatrix, projMatrix, op, mode, worldMatrix);

	// ギズモで編集された場合、transform_に反映
	if (ImGuizmo::IsUsing()) {
		Vector3 scale, rotation, translation;
		this->DecomposeMatrix(worldMatrix, scale, rotation, translation);

		// スケールが0やnanの場合は安全値に補正
		if (std::isnan(scale.x) || std::isnan(scale.y) || std::isnan(scale.z) ||
			scale.x == 0.0f || scale.y == 0.0f || scale.z == 0.0f) {
			scale = Vector3(1.0f, 1.0f, 1.0f);
		}
		transform_.scale = scale;
		transform_.rotate = rotation;
		transform_.translate = translation;
	}
}

void Sprite::DrawImGui() {
	BaseGameObject::DrawImGui();

	ImGui::InputFloat("Width", &width_);
	ImGui::InputFloat("Height", &hight_);
	ImGui::Separator();
	ImGui::Text("Attached Script: %s", attachedScriptName.c_str());
}
#endif // _DEBUG