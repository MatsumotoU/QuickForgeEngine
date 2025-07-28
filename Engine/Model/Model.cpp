#include "Model.h"
#include "ModelManager.h"
#include "../Base/EngineCore.h"
#include "../Base/DirectX/DirectXCommon.h"
#include "../Base/DirectX/TextureManager.h"
#include "../Base/DirectX/DepthStencil.h"

#include "../Camera/Camera.h"

#include "Utility/FileLoader.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

int Model::instanceCount_ = 0;

Model::Model(EngineCore* engineCore, Camera* camera) : BaseGameObject(engineCore, camera) {
	engineCore_ = engineCore;
	modelTextureHandle_ = 0;

	dxCommon_ = engineCore->GetDirectXCommon();
	textureManager_ = engineCore->GetTextureManager();
	pso_ = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);
	worldMatrix_ = Matrix4x4::MakeIndentity4x4();

	// 初期化
	wvp_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	directionalLight_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	material_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());

	assert(dxCommon_);
	assert(textureManager_);
	assert(pso_);

	name_ = "Model"; // モデルの名前を設定
	instanceCount_++;
}

void Model::Init() {
	wvp_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	wvp_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();
	directionalLight_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	directionalLight_.GetData()->direction = Vector3(0.0f, -1.0f, 0.0f); // 下方向
	directionalLight_.GetData()->intensity = 1.0f; // 輝度
	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4(); // UV変換行列を初期化
	material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	material_.GetData()->enableLighting = true; // ライティングを有効にする
}

void Model::Update() {
	// ワールド行列を更新
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

void Model::LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem) {
	std::string instanceNumber = "[" + std::to_string(instanceCount_) + "]";
	name_ = FileLoader::ExtractFileName(filename) + instanceNumber;

	modelData_ = Modelmanager::LoadObjFile(directoryPath, filename, coordinateSystem);
	vertexBuffer_.CreateResource(dxCommon_->GetDevice(), static_cast<uint32_t>(modelData_.vertices.size()));

	std::memcpy(vertexBuffer_.GetData(), modelData_.vertices.data(), sizeof(VertexData) * modelData_.vertices.size());

	// テクスチャを読み込む
	modelTextureHandle_ = textureManager_->LoadTexture(modelData_.material.textureFilePath);
	modelFileName_ = filename;
}

void Model::Draw() {
	Matrix4x4 wvpMatrix = camera_->MakeWorldViewProjectionMatrix(worldMatrix_, CAMERA_VIEW_STATE_PERSPECTIVE);
	wvp_.GetData()->World = worldMatrix_;
	wvp_.GetData()->WVP = wvpMatrix;

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	commandList->RSSetViewports(1, camera_->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera_->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());
	commandList->IASetVertexBuffers(0, 1, vertexBuffer_.GetVertexBufferView());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootConstantBufferView(0, material_.GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(modelTextureHandle_));
	commandList->DrawInstanced(static_cast<UINT>(modelData_.vertices.size()), 1, 0, 0);
}

nlohmann::json Model::Serialize() const {
	nlohmann::json j;
	j["type"] = "Model";
	j["name"] = name_;
	j["position"] = { transform_.translate.x, transform_.translate.y, transform_.translate.z };
	j["rotation"] = { transform_.rotate.x, transform_.rotate.y, transform_.rotate.z };
	j["scale"] = { transform_.scale.x, transform_.scale.y, transform_.scale.z };
	j["modelTextureHandle"] = modelTextureHandle_;
	j["modelFileName"] = modelFileName_;
	j["color"] = { material_.GetData()->color.x, material_.GetData()->color.y, material_.GetData()->color.z, material_.GetData()->color.w };
	j["enableLighting"] = static_cast<bool>(material_.GetData()->enableLighting);
#ifdef _DEBUG
	DebugLog(std::format("name: {}", name_));
#endif // _DEBUG
	return j;
}

std::unique_ptr<Model> Model::Deserialize(const nlohmann::json& j, EngineCore* engineCore, Camera* camera) {
	auto model = std::make_unique<Model>(engineCore, camera);
	model.get()->Init();
	// 名前復元
	if (j.contains("name")) model->name_ = j["name"].get<std::string>();
	// トランスフォーム復元
	if (j.contains("position")) {
		model->transform_.translate.x = j["position"][0].get<float>();
		model->transform_.translate.y = j["position"][1].get<float>();
		model->transform_.translate.z = j["position"][2].get<float>();
	}
	if (j.contains("rotation")) {
		model->transform_.rotate.x = j["rotation"][0].get<float>();
		model->transform_.rotate.y = j["rotation"][1].get<float>();
		model->transform_.rotate.z = j["rotation"][2].get<float>();
	}
	if (j.contains("scale")) {
		model->transform_.scale.x = j["scale"][0].get<float>();
		model->transform_.scale.y = j["scale"][1].get<float>();
		model->transform_.scale.z = j["scale"][2].get<float>();
	}
	// テクスチャとモデルファイル名の復元
	if (j.contains("modelTextureHandle")) {
		model->modelTextureHandle_ = j["modelTextureHandle"].get<int>();
	}
	if (j.contains("modelFileName")) {
		model->modelFileName_ = j["modelFileName"].get<std::string>();
	}
	if (j.contains("color")) {
		model->material_.GetData()->color.x = j["color"][0].get<float>();
		model->material_.GetData()->color.y = j["color"][1].get<float>();
		model->material_.GetData()->color.z = j["color"][2].get<float>();
		model->material_.GetData()->color.w = j["color"][3].get<float>();
	}
	if (j.contains("enableLighting")) {
		model->material_.GetData()->enableLighting = j["enableLighting"].get<int32_t>();
	}
	return model;
}

#ifdef _DEBUG
void Model::DrawImGui() {
	ImGui::Text("Model Name: %s", name_.c_str());
	ImGui::Spacing();
	// 位置情報
	ImGui::DragFloat3("Position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("Rotation", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("Scale", &transform_.scale.x, 0.01f);
	ImGui::Spacing();

	// ワールド行列の表示
	if (ImGui::TreeNode("WorldMatrix")) {
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[0][0], worldMatrix_.m[0][1], worldMatrix_.m[0][2], worldMatrix_.m[0][3]);
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[1][0], worldMatrix_.m[1][1], worldMatrix_.m[1][2], worldMatrix_.m[1][3]);
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[2][0], worldMatrix_.m[2][1], worldMatrix_.m[2][2], worldMatrix_.m[2][3]);
		ImGui::Text("  %f, %f, %f, %f", worldMatrix_.m[3][0], worldMatrix_.m[3][1], worldMatrix_.m[3][2], worldMatrix_.m[3][3]);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Material")) {
		ImGui::ColorEdit4("Color", &material_.GetData()->color.x);
		bool temp = static_cast<bool>(material_.GetData()->enableLighting);
		ImGui::Checkbox("Enable Lighting", &temp);
		material_.GetData()->enableLighting = temp;
		ImGui::TreePop();
	}

	// 詳細
	if (ImGui::TreeNode("Properties")) {
		ImGui::Text("ObjectName: %s", name_.c_str());
		ImGui::Text("Vertex Count: %zu", modelData_.vertices.size());
		ImGui::Text("TextureHandle: %d", modelTextureHandle_);
		ImGui::TreePop();
	}
}
#endif // _DEBUG

void Model::SetBlendmode(BlendMode mode) {
	pso_ = engineCore_->GetGraphicsCommon()->GetTrianglePso(mode);
}
