#include "AssetManager.h"
#include "Graphic/DirectXCommon/DirectXCommon.h"

void AssetManager::Initalize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;
	textureManager_.Initialize(dxCommon_->GetDevice(), dxCommon_->GetCommandManager(D3D12_COMMAND_LIST_TYPE_DIRECT));
}

void AssetManager::Finalize() {
	textureManager_.Finalize();
}

void AssetManager::EndFrame() {
	textureManager_.ReleaseIntermediateResources();
}
