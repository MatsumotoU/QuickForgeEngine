#include "Model.h"
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
	modelTextureHandles_.clear();
	vertexBuffers_.clear();

	dxCommon_ = engineCore->GetDirectXCommon();
	textureManager_ = engineCore->GetTextureManager();
	pso_ = engineCore->GetGraphicsCommon()->GetTrianglePso(kBlendModeNormal);
	worldMatrix_ = Matrix4x4::MakeIndentity4x4();

	// 初期化
	wvp_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	directionalLight_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());
	material_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());

	wvp_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	wvp_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();
	directionalLight_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	directionalLight_.GetData()->direction = Vector3(0.0f, -1.0f, 0.0f); // 下方向
	directionalLight_.GetData()->intensity = 1.0f; // 輝度
	material_.GetData()->uvTransform = Matrix4x4::MakeIndentity4x4(); // UV変換行列を初期化
	material_.GetData()->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // 白色
	material_.GetData()->enableLighting = true; // ライティングを有効にする

	assert(dxCommon_);
	assert(textureManager_);
	assert(pso_);

	name_ = "Model"; // モデルの名前を設定
	type_ = "Model"; // オブジェクトのタイプを設定
	instanceCount_++;

	attachedScriptName.clear();

	viewState_ = ViewState::CAMERA_VIEW_STATE_PERSPECTIVE;
}

void Model::Init() {
	timeCounter_ = 0.0f;
}

void Model::Update() {
	timeCounter_ += engineCore_->GetDeltaTime();
	// ワールド行列を更新
	worldMatrix_ = Matrix4x4::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

void Model::LoadModel(const std::string& directoryPath, const std::string& filename, CoordinateSystem coordinateSystem) {
	std::string instanceNumber = "[" + std::to_string(instanceCount_) + "]";
	name_ = FileLoader::ExtractFileName(filename) + instanceNumber;

	modelData_ = AssimpModelLoader::LoadModelData(directoryPath, filename, coordinateSystem);

	vertexBuffers_.clear();
	modelTextureHandles_.clear();

	for (const auto& mesh : modelData_.meshes) {
		// 頂点バッファ作成
		VertexBuffer<VertexData> vb;
		vb.CreateResource(dxCommon_->GetDevice(), static_cast<uint32_t>(mesh.vertices.size()));
		std::memcpy(vb.GetData(), mesh.vertices.data(), sizeof(VertexData) * mesh.vertices.size());
		vertexBuffers_.push_back(std::move(vb));

		// テクスチャ読み込み
		int32_t texHandle = textureManager_->LoadTexture(mesh.material.textureFilePath);
		modelTextureHandles_.push_back(texHandle);
	}

	modelFileName_ = filename;
}

void Model::Draw() {
	Matrix4x4 wvpMatrix = camera_->MakeWorldViewProjectionMatrix(worldMatrix_, viewState_);
	wvp_.GetData()->World = worldMatrix_;
	wvp_.GetData()->WVP = wvpMatrix;

	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	commandList->RSSetViewports(1, camera_->viewport_.GetViewport());
	commandList->RSSetScissorRects(1, camera_->scissorrect_.GetScissorRect());
	commandList->SetGraphicsRootSignature(pso_->GetRootSignature());
	commandList->SetPipelineState(pso_->GetPipelineState());

	for (size_t i = 0; i < modelData_.meshes.size(); ++i) {
		commandList->IASetVertexBuffers(0, 1, vertexBuffers_[i].GetVertexBufferView());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetGraphicsRootConstantBufferView(0, material_.GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(1, wvp_.GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(3, directionalLight_.GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, textureManager_->GetTextureSrvHandleGPU(modelTextureHandles_[i]));
		commandList->DrawInstanced(static_cast<UINT>(modelData_.meshes[i].vertices.size()), 1, 0, 0);
	}
}

nlohmann::json Model::Serialize() const {
	nlohmann::json j;
	j["type"] = type_;
	j["name"] = name_;
	j["position"] = { transform_.translate.x, transform_.translate.y, transform_.translate.z };
	j["rotation"] = { transform_.rotate.x, transform_.rotate.y, transform_.rotate.z };
	j["scale"] = { transform_.scale.x, transform_.scale.y, transform_.scale.z };
	j["modelFileName"] = modelFileName_;
	j["color"] = { material_.GetData()->color.x, material_.GetData()->color.y, material_.GetData()->color.z, material_.GetData()->color.w };
	j["enableLighting"] = static_cast<bool>(material_.GetData()->enableLighting);
	// メッシュごとのテクスチャハンドルと頂点数を配列で保存
	j["modelTextureHandles"] = modelTextureHandles_;
	std::vector<size_t> meshVertexCounts;
	for (const auto& mesh : modelData_.meshes) {
		meshVertexCounts.push_back(mesh.vertices.size());
	}
	j["meshVertexCounts"] = meshVertexCounts;
	j["scriptFileName"] = GetAttachedScriptName();

	// コライダー情報を保存
	j["radius"] = GetRadius();
	j["mask"] = GetMask();

#ifdef _DEBUG
	DebugLog(std::format("name: {}", name_));
#endif // _DEBUG
	return j;
}

std::unique_ptr<Model> Model::Deserialize(const nlohmann::json& j, EngineCore* engineCore, Camera* camera) {
	auto model = std::make_unique<Model>(engineCore, camera);
	model->Init();
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
	// モデルファイル名の復元
	if (j.contains("modelFileName")) {
		model->modelFileName_ = j["modelFileName"].get<std::string>();
	}
	// テクスチャハンドル（vector）復元
	if (j.contains("modelTextureHandles")) {
		model->modelTextureHandles_ = j["modelTextureHandles"].get<std::vector<int32_t>>();
	}
	// マテリアル情報
	if (j.contains("color")) {
		model->material_.GetData()->color.x = j["color"][0].get<float>();
		model->material_.GetData()->color.y = j["color"][1].get<float>();
		model->material_.GetData()->color.z = j["color"][2].get<float>();
		model->material_.GetData()->color.w = j["color"][3].get<float>();
	}
	if (j.contains("enableLighting")) {
		model->material_.GetData()->enableLighting = j["enableLighting"].get<bool>();
	}

	// スクリプト名の復元
	if (j.contains("scriptFileName")) {
		model->SetScriptName(j["scriptFileName"].get<std::string>());
	}

	// コライダー情報の復元
	if (j.contains("radius")) {
		model->SetRadius(j["radius"].get<float>());
	}
	if (j.contains("mask")) {
		model->SetMask(j["mask"].get<uint32_t>());
	}
	return model;
}

#ifdef _DEBUG
void Model::DrawImGui() {
	BaseGameObject::DrawImGui();

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
		ImGui::Text("Mesh Count: %zu", modelData_.meshes.size());
		for (size_t i = 0; i < modelData_.meshes.size(); ++i) {
			ImGui::Separator();
			ImGui::Text("Mesh[%zu]", i);
			ImGui::Text("  Vertex Count: %zu", modelData_.meshes[i].vertices.size());
			if (i < modelTextureHandles_.size()) {
				ImGui::Text("  TextureHandle: %d", modelTextureHandles_[i]);
			}
			ImGui::Text("  TexturePath: %s", modelData_.meshes[i].material.textureFilePath.c_str());
		}
		ImGui::TreePop();
	}
}
#endif // _DEBUG

void Model::SetBlendmode(BlendMode mode) {
	pso_ = engineCore_->GetGraphicsCommon()->GetTrianglePso(mode);
}

void Model::SetColor(const Vector4& color) {
	material_.GetData()->color = color;
}

void Model::SetDirectionalLightDir(const Vector3& dir) {
	directionalLight_.GetData()->direction = Vector3::Normalize(dir);
}

void Model::SetDirectionalLightIntensity(float intensity) {
	directionalLight_.GetData()->intensity = intensity;
}

void Model::SetViewState(ViewState state) {
	viewState_ = state;
}

void Model::SetEnableShadow(bool enable) {
	material_.GetData()->enableLighting = enable;
}
