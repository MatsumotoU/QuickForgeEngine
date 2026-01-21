/**
 * @file AssetManager.h
 * @brief 各種アセット（テクスチャ、モデル、音など）の一括管理を行うクラス
 */

#pragma once
#include "ResourceDirectoryManager.h"
#include "2DTexture/TextureManager.h"

#include "3DModel/ModelRenderDataManager.h"
#include "3DModel/ModelVertexResourceManager.h"
#include "Sprite/SpriteManager.h" 
#include "Sprite/Data/SpriteData.h"
#include "ConstantBufferManager/ConstantBufferManager.h"
#include "Particle/ParticleGpuDataManager.h"
#include "engine/resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/utility/DesignPatterns/Singleton.h"
#include "AudioSource/AudioSourceManager.h"

#include "Engine/include/graphic/GpuBufferPool/GpuBufferPool.h"

#include <unordered_map>

class DirectXCommon;

/**
 * @class AssetManager
 * @brief テクスチャ、モデル、音声などのアセット読み込みとライフサイクルを管理するシングルトンクラス
 */
class AssetManager final :public Singleton<AssetManager> {
	friend class Singleton<AssetManager>;
	AssetManager() = default;
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

public:
    /** @brief 初期化処理 */
	void Initialize(DirectXCommon* dxCommon);
    /** @brief 描画前処理 */
	void PreDraw();
    /** @brief フレーム終了時の処理 */
	void EndFrame();
    /** @brief 終了処理 */
	void Finalize();
	
	/**
     * @brief テクスチャを読み込む
     * @param imageName ファイル名（拡張子を含む）
     * @return テクスチャハンドル
     */
	uint32_t LoadTexture(const std::string& imageName);
	/**
     * @brief モデルを読み込む
     * @param modelName ファイル名（拡張子を含む）
     * @return モデルハンドル
     */
	uint32_t LoadModel(const std::string& modelName);
	/**
     * @brief 音声を読み込む
     * @param audioName ファイル名（拡張子を含む）
     * @return オーディオハンドル
     */
	uint32_t LoadAudio(const std::string& audioName);

	uint32_t LoadModelMesh(const std::string& modelName);
	uint32_t LoadModelTexture(const std::string& modelName);

#ifdef QFE_OPTIMIZE_OFF
	uint32_t LoadEditorTexture(const std::string& imageName);
#endif // QFE_OPTIMIZE_OFF

	ModelRenderData* GetModelRenderData(uint32_t modelHandle);
	TextureManager* GetTextureManager() { return textureManager_; }

	ModelVertexResourceManager* GetModelVertexResourceManager() { return &modelVertexResourceManager_; }
	GpuBufferPool* GetGpuBufferPool() { return gpuBufferPool_.get(); }
	EntityManager* GetEntityManager() { return &entityManager_; }
	SpriteManager* GetSpriteManager() { return &spriteManager_; }
	const ResourceDirectoryManager* GetResourceDirectoryManager() { return &resourceDirectoryManager_; }
	AudioSourceManager* GetAudioSourceManager() { return &audioSourceManager_; }
	ParticleGpuDataManager* GetParticleGpuDataManager() { return &particleGpuDataManager_; }

private:
	DirectXCommon* dxCommon_;
	ResourceDirectoryManager resourceDirectoryManager_;
	TextureManager* textureManager_;
	AudioSourceManager audioSourceManager_;
	ModelRenderDataManager modelRenderDataManager_;
	ModelVertexResourceManager modelVertexResourceManager_;
	std::unique_ptr<GpuBufferPool> gpuBufferPool_;
	EntityManager entityManager_;
	SpriteManager spriteManager_;
	ParticleGpuDataManager particleGpuDataManager_;
};
