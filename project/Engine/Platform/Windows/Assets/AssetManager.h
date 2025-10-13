#pragma once
#include "ResourceDirectoryManager.h"
#include "2DTexture/TextureManager.h"

#include "3DModel/ModelRenderDataManager.h"
#include "3DModel/ModelVertexResourceManager.h"
#include "Sprite/SpriteManager.h" 
#include "Sprite/Data/SpriteData.h"
#include "ConstantBufferManager/ConstantBufferManager.h"
#include "Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"
#include "Core/Entity/EntityManager.h"
#include "Utility/DesignPatterns/Singleton.h"

#include <unordered_map>

class DirectXCommon;

class AssetManager final :public Singleton<AssetManager> {
	friend class Singleton<AssetManager>;
	AssetManager() = default;
	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

public:
	void Initalize(DirectXCommon* dxCommon);
	void PreDraw();
	void EndFrame();
	void Finalize();
	
	/// 拡張子付きで書くこと
	uint32_t LoadTexture(const std::string& imageName);
	/// 拡張子付きで書くこと
	uint32_t LoadModel(const std::string& modelName);

#ifdef _DEBUG
	uint32_t LoadEditorTexture(const std::string& imageName);
#endif // _DEBUG

	ModelRenderData* GetModelRenderData(uint32_t modelHandle);
	TextureManager* GetTextureManager() { return textureManager_; }
	ModelVertexResourceManager* GetModelVertexResourceManager() { return &modelVertexResourceManager_; }
	ConstantBufferManager<TransformationMatrix>* GetWpvBufferManager() { return &wpvBufferManager_; }
	ConstantBufferManager<Material>* GetMaterialBufferManager() { return &materialBufferManager_; }
	ConstantBufferManager<DirectionalLight>* GetLightBufferManager() { return &lightBufferManager_; }
	EntityManager* GetEntityManager() { return &entityManager_; }
	SpriteManager* GetSpriteManager() { return &spriteManager_; }
	const ResourceDirectoryManager* GetResourceDirectoryManager() { return &resourceDirectoryManager_; }

private:
	DirectXCommon* dxCommon_;
	ResourceDirectoryManager resourceDirectoryManager_;

	TextureManager* textureManager_;

	ModelRenderDataManager modelRenderDataManager_;
	ModelVertexResourceManager modelVertexResourceManager_;
	ConstantBufferManager<TransformationMatrix> wpvBufferManager_;
	ConstantBufferManager<Material> materialBufferManager_;
	ConstantBufferManager<DirectionalLight> lightBufferManager_;
	EntityManager entityManager_;
	SpriteManager spriteManager_;
};