#include "FileUtility.h"
#include <cassert>
#include <windows.h>

std::vector<std::string> QFE::FILE::GetFilesInDirectory(const std::string& directoryPath, const std::string& extension) {
    std::vector<std::string> files;
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            if (extension.empty() || entry.path().extension() == extension) {
                files.push_back(entry.path().filename().string());
            }
        }
    }
    return files;
}

bool QFE::FILE::OpenFileOnExe(const std::string& exePath, const std::string& filePath) {
    // ShellExecuteAの戻り値ぁE2以下なら失敁E
    HINSTANCE result = ShellExecuteA(
        NULL,           // ウィンドウハンドル
        "open",         // 操佁E
        exePath.c_str(),// 実行するexe
        filePath.c_str(),// 引数�E�ここでは開きたいファイルパス�E�E
        NULL,           // カレントディレクトリ
        SW_SHOWNORMAL   // ウィンドウ表示方況E
    );
    return reinterpret_cast<intptr_t>(result) > 32;
}

bool QFE::FILE::LoadFileToJson(const std::string& filePath, nlohmann::json& json) {
	std::ifstream ifs(filePath);
    if (ifs.is_open()) {
        try {
            ifs >> json;
            ifs.close();
            return true;
        }
        catch (const nlohmann::json::parse_error& e) {
            ifs.close();
            e;
            assert(false && e.what());
        }
    }
    return false;
}

bool QFE::FILE::HasExtension(const std::string& fileName, const std::string& extension) {
    if (fileName.empty() || extension.empty()) {
        return false;
    }
    return fileName.size() >= extension.size() &&
        fileName.compare(fileName.size() - extension.size(), extension.size(), extension) == 0;
}

bool QFE::FILE::LoadCSVToVector(const std::string& filePath, std::vector<std::vector<uint32_t>>& map) {
	std::ifstream ifs(filePath);
	if (ifs.is_open()) {
		std::string line;
		while (std::getline(ifs, line)) {
			std::istringstream ss(line);
			std::string cell;
			std::vector<uint32_t> row;
			while (std::getline(ss, cell, ',')) {
				row.push_back(static_cast<uint32_t>(std::stoul(cell)));
			}
			map.push_back(row);
		}
		ifs.close();
		return true;
	}
    return false;
}

bool QFE::FILE::SaveJSONToFile(const std::string& filePath, const nlohmann::json& json) {
	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		ofs << json.dump(4); // インチE��ト幁Eで整形して保孁E
		ofs.close();
		return true;
	}
    return false;
}
