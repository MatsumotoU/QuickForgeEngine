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
    // ShellExecuteAの戻り値が32以下なら失敗
    HINSTANCE result = ShellExecuteA(
        NULL,           // ウィンドウハンドル
        "open",         // 操作
        exePath.c_str(),// 実行するexe
        filePath.c_str(),// 引数（ここでは開きたいファイルパス）
        NULL,           // カレントディレクトリ
        SW_SHOWNORMAL   // ウィンドウ表示方法
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
            assert(false && e.what());
        }
    }
    return false;
}
