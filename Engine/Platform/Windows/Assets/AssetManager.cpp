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
}

void AssetManager::Finalize() {
	textureManager_->Finalize();
}

void AssetManager::LoadTexture(const std::string& imageName) {
	std::string filePath = resourceDirectoryManager_.GetResourceDirectory("Image") + imageName;
	textureHandleMap_[imageName] = textureManager_->LoadTexture(filePath);
}

void AssetManager::LoadModel(const std::string& modelName) {
	ModelData modelData;
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"), resourceDirectoryManager_.GetResourceDirectory("Image"),modelName,modelData);
	LoadTexture(modelName);

	modelVertexResourceManager_.AssignModelVertexBuffer(dxCommon_->GetDevice(), modelName,modelData);
}

D3D12_GPU_DESCRIPTOR_HANDLE AssetManager::GetTextureSrvHandleGPU(const std::string& imageName) {
	assert(textureHandleMap_.find(imageName) != textureHandleMap_.end() && "Not Found Image.");
	return textureManager_->GetTextureSrvHandleGPU(textureHandleMap_[imageName]);
}

void AssetManager::EndFrame() {
	textureManager_->ReleaseIntermediateResources();
}
