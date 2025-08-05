#include "Asset.h"
#include <fstream>
#include <cassert>

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG


void Asset::SaveMetadata(const std::string& filePath) {
	// メタデータを指定されたファイルパスに保存します
	std::ofstream file(filePath);
	if (file.is_open()) {
		file << metadata_.dump(4); // 4はインデントのスペース数
		file.close();
	} else {
		assert(false && "Failed to open file for saving metadata");
	}
}

void Asset::LoadMetadata(const std::string& filePath) {
#ifdef _DEBUG
	DebugLog("Loading asset metadata from: " + filePath);
#endif // _DEBUG

	std::ifstream file(filePath);
	if (file.is_open()) {
		nlohmann::json metadata;
		file >> metadata;
		file.close();
		metadata_ = metadata; // 読み込んだメタデータを保存
	} else {
		assert(false && "Failed to open file for loading metadata");
		metadata_ = nlohmann::json(); // エラー時は空のjsonを設定
	}

	name_ = filePath.substr(filePath.find_last_of("/\\") + 1); // ファイル名を抽出
}
