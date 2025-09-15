#include "pch.h"
#include "AssetManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"
#include "Assets/3DModel/Loader/AssimpModelLoader.h"

void AssetManager::Initalize(DirectXCommon* dxCommon) {
	assert(dxCommon && "dxCommon is nullptr.");

	dxCommon_ = dxCommon;
	textureManager_ = TextureManager::GetInstance();
	textureManager_->Initialize(
		dxCommon_->GetDevice(), dxCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT),
		dxCommon_->GetDescriptorHeapManager()->GetSrvDescriptorHeap());

	wpvBufferManager_.Initialize();
	materialBufferManager_.Initialize();
	lightBufferManager_.Initialize();
	modelVertexResourceManager_.Initialize();
	modelRenderDataManager_.Initialize();
}

void AssetManager::Finalize() {
	textureManager_->Finalize();
	wpvBufferManager_.Finalize();
	materialBufferManager_.Finalize();
	lightBufferManager_.Finalize();
	modelVertexResourceManager_.Finalize();
	modelRenderDataManager_.Finalize();
}

uint32_t AssetManager::LoadTexture(const std::string& imageName) {
	std::string filePath = resourceDirectoryManager_.GetResourceDirectory("Image") + imageName;
	return textureManager_->LoadTexture(filePath);
}

uint32_t AssetManager::LoadModel(const std::string& modelName) {
	ModelRenderData modelRenderData;

	// モデル自体の読み込み
	ModelData modelData{};
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName,modelData);

	// メッシュの数だけメッシュ描画データを確保
	modelRenderData.meshRenderDataHandles.resize(modelData.meshes.size());
	modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle = modelVertexResourceManager_.Assign(dxCommon_->GetDevice(), modelData);

	// 各メッシュの描画データを作成
	for (size_t i = 0; i < modelData.meshes.size(); i++) {
		auto& mesh = modelData.meshes.at(i);
		auto& meshRenderData = modelRenderData.meshRenderDataHandles.at(i);
		meshRenderData.vertexBufferHandle = modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle + static_cast<uint32_t>(i);
		meshRenderData.textureHandle = textureManager_->LoadTexture(mesh.material.textureFilePath);
		meshRenderData.materialHandle = materialBufferManager_.CreateBuffer();
		meshRenderData.wpvBufferHandle = wpvBufferManager_.CreateBuffer();
		meshRenderData.lightBufferHandle = lightBufferManager_.CreateBuffer();
	}
	
	// モデル描画データを登録
	return modelRenderDataManager_.Add(modelRenderData);
}

const ModelRenderData* AssetManager::GetModelRenderData(uint32_t modelHandle) const {
	return modelRenderDataManager_.Get(modelHandle);
}

void AssetManager::EndFrame() {
	textureManager_->ReleaseIntermediateResources();
}
