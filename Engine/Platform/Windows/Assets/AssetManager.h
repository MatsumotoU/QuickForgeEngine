#pragma once
#include "ResourceDirectoryManager.h"
#include "2DTexture/TextureManager.h"
#include "3DModel/ModelVertexResourceManager.h"

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
	void EndFrame();
	void Finalize();
	
	void LoadTexture(const std::string& imageName);
	void LoadModel(const std::string& modelName);

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureSrvHandleGPU(const std::string& imageName);

private:
	DirectXCommon* dxCommon_;

	ResourceDirectoryManager resourceDirectoryManager_;
	TextureManager* textureManager_;
	std::unordered_map<std::string, uint32_t> textureHandleMap_;
	ModelVertexResourceManager modelVertexResourceManager_;
};