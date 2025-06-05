#include "SimpleJson.h"
#include <cassert>

SJN::json SJN::LoadJsonData(std::string& fp) {
    SJN::json result;
    
	// jsonデータ読み込み
	std::ifstream ifs;
	ifs.open(fp);

	// ファイル開けない時のエラー
	if (ifs.fail()) {
		assert(false && "Failed open data file for read");
		return 0;
	}

	ifs >> result;
	ifs.close();

    return result;
}

void SJN::SaveJsonData(const std::string groupName, json saveData) {

	// ファイル生成(階層ごとにやらなきゃダメっぽい？)
	if (!std::filesystem::exists("Resources")) {
		std::filesystem::create_directory("Resources");
	}
	if (!std::filesystem::exists("Resources/GlobalVariables")) {
		std::filesystem::create_directory("Resources/GlobalVariables");
	}

	// jsonデータ生成
	std::string filePath = kDirectoryPath + groupName + ".json";
	std::ofstream ofs;
	ofs.open(filePath);

	// ファイル開けない時のエラー
	if (ofs.fail()) {
		assert(false && "Failed open data file for write.");
		return;
	}

	ofs << std::setw(4) << saveData << std::endl;
	ofs.close();
}
