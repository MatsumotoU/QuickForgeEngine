#include "SceneGameObjectGenerator.h"
#include "SceneObject.h"
#include "Object/Asset/AssetManager.h"

SceneGameObjectGenerator::SceneGameObjectGenerator(SceneObject* sceneObj, AssetManager* assetMng) 
	:sceneObject_(sceneObj) , assetManager_(assetMng){
}

void SceneGameObjectGenerator::AddAsset(const std::string& assetName) {
	if (!sceneObject_ || !assetManager_) {
		return; // シーンオブジェクトまたはアセットマネージャーが無効な場合は何もしない
	}
	// アセットを取得
	Asset asset = assetManager_->GetAsset(assetName);
	if (asset.GetMetadata().is_null()) {
		return; // メタデータが無い場合は何もしない
	}
	// メタデータからオブジェクトを生成
	nlohmann::json metadata = asset.GetMetadata();
	//sceneObject_->AddObjectFromJson(metadata);
}
