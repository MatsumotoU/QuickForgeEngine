#include "engine/include/utility/FileSystems/FileUtility.h"
#include "engine/include/utility/String/MyString.h"
#include <cassert>
#include <windows.h>

std::vector<std::string> QFE::FILE::GetFilesInDirectory(const std::string& directoryPath, const std::string& extension) {
    std::vector<std::string> files;
    namespace fs = std::filesystem;

    for (const auto& entry : fs::directory_iterator(fs::path(ConvertString(directoryPath)))) {
        if (entry.is_regular_file()) {
            if (extension.empty() || entry.path().extension() == fs::path(ConvertString(extension))) {
                files.push_back(WideToUTF8(entry.path().filename().wstring()));
            }
        }
    }
    return files;
}

bool QFE::FILE::OpenFileOnExe(const std::string& exePath, const std::string& filePath) {
    const std::wstring executable = ConvertString(exePath);
    const std::wstring file = L"\"" + ConvertString(filePath) + L"\"";
    HINSTANCE result = ShellExecuteW(
        NULL,           // ウィンドウハンドル
        L"open",        // 操作
        executable.c_str(), // 実行するexe
        file.c_str(),    // 引数（ここでは開きたいファイルパス）
        NULL,           // カレントディレクトリ
        SW_SHOWNORMAL   // ウィンドウ表示方法
    );
    return reinterpret_cast<intptr_t>(result) > 32;
}

bool QFE::FILE::LoadFileToJson(const std::string& filePath, nlohmann::json& json) {
	std::ifstream ifs(std::filesystem::path(ConvertString(filePath)));
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
	std::ifstream ifs(std::filesystem::path(ConvertString(filePath)));
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
	std::ofstream ofs(std::filesystem::path(ConvertString(filePath)));
	if (ofs.is_open()) {
		ofs << json.dump(4); // インデント幅4で整形して保存
		ofs.close();
		return true;
	}
    return false;
}

std::string QFE::FILE::WideToUTF8(const std::wstring& wstr)
{
    if (wstr.empty()) return {};
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
    return result;
}
