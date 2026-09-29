#include "engine/include/assets/AssetManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/assets/3DModel/Loader/AssimpModelLoader.h"

#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
void AssetManager::Initalize(DirectXCommon* dxCommon) {
	assert(dxCommon && "dxCommon is nullptr.");

	dxCommon_ = dxCommon;
	textureManager_ = TextureManager::GetInstance();
	textureManager_->Initialize(
		dxCommon_->GetDevice(), dxCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT),
		dxCommon_->GetDescriptorHeapManager()->GetSrvDescriptorHeap());

	modelVertexResourceManager_.Initialize();
	modelRenderDataManager_.Initialize();
	spriteManager_.Initialize();
	audioSourceManager_.Initialize();
	particleGpuDataManager_.Initialize();
	gpuBufferPool_ = std::make_unique<GpuBufferPool>(dxCommon);
}

void AssetManager::PreDraw() {
	
}
void AssetManager::Finalize() {
	particleGpuDataManager_.Finalize();
	audioSourceManager_.Finalize();
	spriteManager_.Finalize();
	textureManager_->Finalize();
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
		modelName, modelData);

	// メッシュの数だけメッシュ描画データを確保
	modelRenderData.meshRenderDataHandles.resize(modelData.meshes.size());
	modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle = modelVertexResourceManager_.Assign(dxCommon_->GetDevice(), modelData, modelName);

	// 各メッシュの描画データを作成
	for (size_t i = 0; i < modelData.meshes.size(); i++) {
		auto& mesh = modelData.meshes.at(i);
		auto& meshRenderData = modelRenderData.meshRenderDataHandles.at(i);
		meshRenderData.vertexBufferHandle = modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle + static_cast<uint32_t>(i);
		meshRenderData.textureHandle = textureManager_->LoadTexture(mesh.material.textureFilePath);
		meshRenderData.materialHandle = gpuBufferPool_->AcquireConstantBuffer<Material>();
		meshRenderData.wpvBufferHandle = gpuBufferPool_->AcquireConstantBuffer<TransformationMatrix>();
		meshRenderData.lightBufferHandle = gpuBufferPool_->AcquireConstantBuffer<DirectionalLight>();

		Material* materialData = gpuBufferPool_->GetConstantBufferData<Material>(meshRenderData.materialHandle);
		materialData->color = { 1.0f,1.0f,1.0f,1.0f };
		materialData->enableLighting = true;
		materialData->uvTransform = Matrix4x4::MakeIndentity4x4();
		TransformationMatrix* transformData = gpuBufferPool_->GetConstantBufferData<TransformationMatrix>(meshRenderData.wpvBufferHandle);
		transformData->World = Matrix4x4::MakeIndentity4x4();
		transformData->WVP = Matrix4x4::MakeIndentity4x4();
		DirectionalLight* lightData = gpuBufferPool_->GetConstantBufferData<DirectionalLight>(meshRenderData.lightBufferHandle);
		lightData->color = { 1.0f,1.0f,1.0f,1.0f };
		lightData->direction = { 0.0f,-1.0f,0.0f };
		lightData->intensity = 1.0f;
	}

	// モデル描画データを登録
	return modelRenderDataManager_.Add(modelRenderData);
}

uint32_t AssetManager::LoadAudio(const std::string& audioName) {
	std::string filePath = resourceDirectoryManager_.GetResourceDirectory("Sounds") + audioName;
	uint32_t handle = audioSourceManager_.LoadSoundData(filePath);
	return handle;
}

uint32_t AssetManager::LoadModelMesh(const std::string& modelName) {
	// 既に読み込まれている場合はそのハンドルを返す
	if (modelVertexResourceManager_.HasModelHandle(modelName)) {
		return modelVertexResourceManager_.GetModelHandle(modelName);
	}

	// モデル自体の読み込み
	ModelData modelData{};
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName, modelData);
	return modelVertexResourceManager_.Assign(dxCommon_->GetDevice(), modelData, modelName);
}

uint32_t AssetManager::LoadModelTexture(const std::string& modelName) {
	// モデルデータを読み込み
	ModelData modelData{};
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName, modelData);
	// 先頭のメッシュのテクスチャを返す
	if (!modelData.meshes.empty()) {
		const auto& mesh = modelData.meshes.at(0);
		return textureManager_->LoadTexture(mesh.material.textureFilePath);
	}
	assert(false && "Model has no meshes.");
	return 0;
}

#ifdef _DEBUG
uint32_t AssetManager::LoadEditorTexture(const std::string& imageName) {
	std::string filePath = resourceDirectoryManager_.GetResourceDirectory("Editor") + imageName;
	return textureManager_->LoadTexture(filePath);
}
#endif // _DEBUG

ModelRenderData* AssetManager::GetModelRenderData(uint32_t modelHandle) {
	return modelRenderDataManager_.Get(modelHandle);
}

void AssetManager::EndFrame() {
	textureManager_->ReleaseIntermediateResources();
	entityManager_.EndFrame();
}
