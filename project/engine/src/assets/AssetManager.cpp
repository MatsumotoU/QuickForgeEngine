#include "AssetManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/assets/3DModel/Loader/AssimpModelLoader.h"
#include "Script/CsharpCmpiler.h"

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
	spriteManager_.Initialize();
	audioSourceManager_.Initialize();
	particleGpuDataManager_.Initialize();
}

void AssetManager::PreDraw() {
	
}

void AssetManager::Finalize() {
	particleGpuDataManager_.Finalize();
	audioSourceManager_.Finalize();
	spriteManager_.Finalize();
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

	// 繝｢繝・Ν閾ｪ菴薙・隱ｭ縺ｿ霎ｼ縺ｿ
	ModelData modelData{};
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName, modelData);

	// 繝｡繝・す繝･縺ｮ謨ｰ縺縺代Γ繝・す繝･謠冗判繝・・繧ｿ繧堤｢ｺ菫・
	modelRenderData.meshRenderDataHandles.resize(modelData.meshes.size());
	modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle = modelVertexResourceManager_.Assign(dxCommon_->GetDevice(), modelData, modelName);

	// 蜷・Γ繝・す繝･縺ｮ謠冗判繝・・繧ｿ繧剃ｽ懈・
	for (size_t i = 0; i < modelData.meshes.size(); i++) {
		auto& mesh = modelData.meshes.at(i);
		auto& meshRenderData = modelRenderData.meshRenderDataHandles.at(i);
		meshRenderData.vertexBufferHandle = modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle + static_cast<uint32_t>(i);
		meshRenderData.textureHandle = textureManager_->LoadTexture(mesh.material.textureFilePath);
		meshRenderData.materialHandle = materialBufferManager_.CreateBuffer();
		meshRenderData.wpvBufferHandle = wpvBufferManager_.CreateBuffer();
		meshRenderData.lightBufferHandle = lightBufferManager_.CreateBuffer();

		materialBufferManager_.GetBufferData(meshRenderData.materialHandle)->color = { 1.0f,1.0f,1.0f,1.0f };
		materialBufferManager_.GetBufferData(meshRenderData.materialHandle)->enableLighting = true;
		materialBufferManager_.GetBufferData(meshRenderData.materialHandle)->uvTransform = Matrix4x4::MakeIndentity4x4();
		wpvBufferManager_.GetBufferData(meshRenderData.wpvBufferHandle)->World = Matrix4x4::MakeIndentity4x4();
		wpvBufferManager_.GetBufferData(meshRenderData.wpvBufferHandle)->WVP = Matrix4x4::MakeIndentity4x4();
		lightBufferManager_.GetBufferData(meshRenderData.lightBufferHandle)->color = { 1.0f,1.0f,1.0f,1.0f };
		lightBufferManager_.GetBufferData(meshRenderData.lightBufferHandle)->direction = { 0.0f,-1.0f,0.0f };
		lightBufferManager_.GetBufferData(meshRenderData.lightBufferHandle)->intensity = 1.0f;
	}

	// 繝｢繝・Ν謠冗判繝・・繧ｿ繧堤匳骭ｲ
	return modelRenderDataManager_.Add(modelRenderData);
}

uint32_t AssetManager::LoadAudio(const std::string& audioName) {
	std::string filePath = resourceDirectoryManager_.GetResourceDirectory("Sounds") + audioName;
	uint32_t handle = audioSourceManager_.LoadSoundData(filePath);
	return handle;
}

uint32_t AssetManager::LoadModelMesh(const std::string& modelName) {
	// 繝｢繝・Ν繝・・繧ｿ繧定ｪｭ縺ｿ霎ｼ縺ｿ
	ModelData modelData{};
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName, modelData);
	// 繝｡繝・す繝･繧定ｿ斐☆
	return modelVertexResourceManager_.Assign(dxCommon_->GetDevice(), modelData, modelName);
}

uint32_t AssetManager::LoadModelTexture(const std::string& modelName) {
	// 繝｢繝・Ν繝・・繧ｿ繧定ｪｭ縺ｿ霎ｼ縺ｿ
	ModelData modelData{};
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName, modelData);
	// 蜈磯ｭ縺ｮ繝｡繝・す繝･縺ｮ繝・け繧ｹ繝√Ε繧定ｿ斐☆
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
