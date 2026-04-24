/**
 * @file AssetManager.cpp
 * @brief アセット全体を統轄管理するクラスの実装
 */

#include "engine/include/assets/AssetManager.h"
#include "engine/include/graphic/DirectXCommon/DirectXCommon.h"
#include "engine/include/assets/3DModel/Loader/AssimpModelLoader.h"
#include "engine/include/assets/Script/MonoRuntimeManager.h"

#include "Engine/Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

using namespace QFE;

/** @brief 初期化 */
void AssetManager::Initialize(DirectXCommon* dxCommon) {
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

	// スクリプトランタイムのグローバル初期化
	MonoRuntimeManager::GetInstance()->Initialize();
}

void AssetManager::PreDraw() {

}
void AssetManager::Finalize() {
	// スクリプトランタイムの終了処理
	MonoRuntimeManager::GetInstance()->Finalize();

	particleGpuDataManager_.Finalize();
	audioSourceManager_.Finalize();
	spriteManager_.Finalize();
	textureManager_->Finalize();
	modelVertexResourceManager_.Finalize();
	modelRenderDataManager_.Finalize();
}

/** @brief テクスチャの読み込み */
uint32_t AssetManager::LoadTexture(const std::string& imageName) {
	std::string filePath = resourceDirectoryManager_.GetResourceDirectory("Image") + imageName;
	return textureManager_->LoadTexture(filePath);
}

/** @brief 3Dモデルの読み込みとレンダリングデータのセットアップ */
uint32_t AssetManager::LoadModel(const std::string& modelName) {
	ModelRenderData modelRenderData;

	// モデル自体の読み込み
	ModelData modelData;
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName, modelData);

	// モデルのデータの整合性を確認
	for(auto& mesh : modelData.meshes) {
		// テクスチャファイルパスが空の場合は読み込めない事にする
		if (mesh.material.textureFilePath.empty()) {
#ifdef QFE_OPTIMIZE_OFF
			DebugLog("Mesh " + mesh.material.textureFilePath + " in model " + modelName + " has no texture file path.", LogLevel::Warning);
#endif // QFE_OPTIMIZE_OFF
			throw std::runtime_error("Mesh " + mesh.material.textureFilePath + " in model " + modelName + " has no texture file path.");
		}
	}

	// メッシュの数だけメッシュ描画データを確保
	modelRenderData.meshRenderDataHandles.resize(modelData.meshes.size());
	// TODO: modelData.meshes が空の場合、以下のアクセスでクラッシュする可能性がある
	if (!modelData.meshes.empty()) {
		modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle = modelVertexResourceManager_.Assign(dxCommon_->GetDevice(), modelData, modelName);
	}

	// 各メッシュの描画データを作成
	for (size_t i = 0; i < modelData.meshes.size(); i++) {
		auto& mesh = modelData.meshes.at(i);
		auto& meshRenderData = modelRenderData.meshRenderDataHandles.at(i);
		meshRenderData.vertexBufferHandle = modelRenderData.meshRenderDataHandles.at(0).vertexBufferHandle + static_cast<uint32_t>(i);
		meshRenderData.textureHandle = textureManager_->LoadTexture(mesh.material.textureFilePath);
		meshRenderData.materialHandle = gpuBufferPool_->AcquireConstantBuffer<Material>();
		meshRenderData.wpvBufferHandle = gpuBufferPool_->AcquireConstantBuffer<TransformationMatrix>();
		meshRenderData.lightBufferHandle = gpuBufferPool_->AcquireConstantBuffer<DirectionalLight>();
		meshRenderData.cameraPosBufferHandle = gpuBufferPool_->AcquireConstantBuffer<CameraForGPU>();

		Material* materialData = gpuBufferPool_->GetConstantBufferData<Material>(meshRenderData.materialHandle);
		materialData->color = { 1.0f,1.0f,1.0f,1.0f };
		materialData->enableLighting = true;
		materialData->uvTransform = QFE::Matrix4x4::MakeIndentity4x4();
		materialData->shininess = 48.0f;
		TransformationMatrix* transformData = gpuBufferPool_->GetConstantBufferData<TransformationMatrix>(meshRenderData.wpvBufferHandle);
		transformData->World = QFE::Matrix4x4::MakeIndentity4x4();
		transformData->WVP = QFE::Matrix4x4::MakeIndentity4x4();
		DirectionalLight* lightData = gpuBufferPool_->GetConstantBufferData<DirectionalLight>(meshRenderData.lightBufferHandle);
		lightData->color = { 1.0f,1.0f,1.0f,1.0f };
		lightData->direction = { 0.0f,-1.0f,0.0f };
		lightData->intensity = 1.0f;
		CameraForGPU* cameraData = gpuBufferPool_->GetConstantBufferData<CameraForGPU>(meshRenderData.cameraPosBufferHandle);
		cameraData->cameraPosition = { 0.0f,0.0f,0.0f };
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
	ModelData modelData;
	AssimpModelLoader::LoadModelData(
		resourceDirectoryManager_.GetResourceDirectory("Model"),
		resourceDirectoryManager_.GetResourceDirectory("Image"),
		modelName, modelData);
	return modelVertexResourceManager_.Assign(dxCommon_->GetDevice(), modelData, modelName);
}

uint32_t AssetManager::LoadModelTexture(const std::string& modelName) {
	// 繝｢繝・Ν繝・・繧ｿ繧定ｪｭ縺ｿ霎ｼ縺ｿ
	ModelData modelData;
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

#ifdef QFE_OPTIMIZE_OFF
uint32_t AssetManager::LoadEditorTexture(const std::string& imageName) {
	std::string filePath = resourceDirectoryManager_.GetEditorResourceDirectory() + imageName;
	return textureManager_->LoadTexture(filePath);
}
#endif // QFE_OPTIMIZE_OFF

ModelRenderData* AssetManager::GetModelRenderData(uint32_t modelHandle) {
	return modelRenderDataManager_.Get(modelHandle);
}

void AssetManager::EndFrame() {
	textureManager_->ReleaseIntermediateResources();
}
