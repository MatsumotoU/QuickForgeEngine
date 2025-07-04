#include "SimpleJson.h"

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
#endif // _DEBUG

nlohmann::json SJN::LoadJsonData(const std::string& groupName) {
	nlohmann::json result;
	std::string fp = kDirectoryPath + groupName + ".json";

	// jsonデータ読み込み
	std::fstream file;
	file.open(fp, std::ios::in | std::ios::out);

	// ファイル開いたらデータを移す
	if (file.is_open()) {
		file >> result;
		file.close();
#ifdef _DEBUG
		std::string message = "Read(" + fp + ")";
		DebugLog(message);
#endif // _DEBUG

	} else {
		// ファイルが存在しない場合生成
		SaveJsonData(groupName, result);
#ifdef _DEBUG
		std::string message = "Failed open data file for read(" + fp + ")";
		DebugLog(message);
#endif // _DEBUG
		
	}

    return result;
}

void SJN::SaveJsonData(const std::string groupName, nlohmann::json saveData) {

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
#ifdef _DEBUG
		std::string message = "Failed open data file for write.";
		DebugLog(message);
		assert(false);
#endif // _DEBUG
		
		return;
	}

	ofs << std::setw(4) << saveData << std::endl;
	ofs.close();
}

int SJN::ChackJsonfile(std::string fp) {
	// jsonデータ読み込み
	std::ifstream ifs;
	ifs.open(fp);

	// ファイルが開けたらtrue
	if (ifs.fail()) {
		ifs.close();
		return false;
	} else {
		ifs.close();
		return true;
	}
}
