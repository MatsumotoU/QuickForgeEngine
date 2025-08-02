#pragma once
#include "Asset.h"
#include <string>
#include <vector>

class BaseGameObject;

class AssetManager {
public:
	AssetManager();

public:
	void SaveAssets();
	void AddAsset(const BaseGameObject& obj);
	Asset GetAsset(const std::string& name);
	std::string GetDirectoryPath() const {
		return directoryPath_;
	}
	std::vector<std::string> GetAssetFiles() const {
		return assetFiles_;
	}

#ifdef _DEBUG
	void DrawImGui();
#endif // _DEBUG

	int selectedAssetIndex_ = -1; // 選択されたアセットのインデックス
private:
	void LoadAssetsFromDirectory();
	void SaveAssetsToDirectory();

private:
	std::vector<std::string> assetFiles_;
	std::string directoryPath_;
	std::vector<Asset> assets_;
	
};