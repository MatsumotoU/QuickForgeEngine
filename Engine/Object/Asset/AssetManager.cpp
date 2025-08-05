#include "AssetManager.h"
#include "Utility/FileLoader.h"

#include "Object/BaseGameObject.h"
#include <assert.h>
#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

AssetManager::AssetManager() {
	directoryPath_ = "Resources/Assets"; // デフォルトのアセットディレクトリパス
	assets_.clear(); // アセットの初期化
	assetFiles_.clear(); // アセットファイルの初期化
	LoadAssetsFromDirectory(); // コンストラクタでアセットを読み込みます
}

void AssetManager::SaveAssets() {
	// アセットのメタデータを指定されたディレクトリに保存します
	if (assets_.empty()) {
		LoadAssetsFromDirectory(); // アセットが空の場合はディレクトリから読み込み
	}
	SaveAssetsToDirectory();
}

void AssetManager::AddAsset(const BaseGameObject& obj) {
	// 新しいアセットを作成し、名前とメタデータを設定します
	Asset newAsset;
	newAsset.SetName(obj.GetName());
	newAsset.SetMetadata(obj.Serialize()); // BaseGameObjectのSerializeメソッドを使用してメタデータを取得
	assets_.push_back(newAsset);
	SaveAssetsToDirectory();
}

Asset AssetManager::GetAsset(const std::string& name) {
	// 名前でアセットを検索し、見つかった場合は返します
	for (const auto& asset : assets_) {
		if (asset.GetName() == name) {
			return asset;
		}
	}
	assert(false && "Asset not found in AssetManager::GetAsset by name");
	return Asset();
}

#ifdef _DEBUG
void AssetManager::DrawImGui() {
	// アセットマネージャーのImGuiウィンドウを描画します
	ImGui::Begin("Asset Manager");
	ImGui::Text("Assets Directory: %s", directoryPath_.c_str());
	ImGui::Spacing();
	if (ImGui::Button("Save Assets")) {
		SaveAssets();
	}
	if (ImGui::Button("Load Assets")) {
		LoadAssetsFromDirectory();
	}
	ImGui::Separator();
	ImGui::Text("Assets List:");
	for (const auto& asset : assets_) {
		ImGui::Text("%s", asset.GetName().c_str());
	}
	ImGui::End();
}
#endif // _DEBUG

void AssetManager::LoadAssetsFromDirectory() {
	// ディレクトリ内のアセットjsonファイルをすべて格納します
	assetFiles_ = FileLoader::GetFilesWithExtension(directoryPath_, ".json");

#ifdef _DEBUG
	DebugLog(std::format("Loading assets from directory: %s", directoryPath_.c_str()));
#endif // _DEBUG

	for (const auto& file : assetFiles_) {
		std::string filePath = directoryPath_ + "/" + file;
		Asset asset;
		asset.LoadMetadata(filePath);
		assets_.push_back(asset); // 読み込んだアセットを追加
	}
}

void AssetManager::SaveAssetsToDirectory() {
	// アセットのメタデータを指定されたディレクトリに保存します
	for (auto& asset : assets_) {
		std::string filePath = directoryPath_ + "/" + asset.GetName() + ".json";
		asset.SaveMetadata(filePath);
	}
}
