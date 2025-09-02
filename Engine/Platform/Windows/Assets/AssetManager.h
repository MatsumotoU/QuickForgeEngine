#pragma once
#include "ResourceDirectoryManager.h"
#include "2DTexture/TextureManager.h"

class DirectXCommon;

class AssetManager final {
public:
	void Initalize(DirectXCommon* dxCommon);
	void Finalize();

	void EndFrame();

private:
	DirectXCommon* dxCommon_;

	ResourceDirectoryManager resourceDirectoryManager_;
	TextureManager textureManager_;
};