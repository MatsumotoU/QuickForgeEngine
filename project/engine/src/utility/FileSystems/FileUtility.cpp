#include "engine/include/utility/FileSystems/FileUtility.h"
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
    // ShellExecuteA縺ｮ謌ｻ繧雁､縺・2莉･荳九↑繧牙､ｱ謨・
    HINSTANCE result = ShellExecuteA(
        NULL,           // 繧ｦ繧｣繝ｳ繝峨え繝上Φ繝峨Ν
        "open",         // 謫堺ｽ・
        exePath.c_str(),// 螳溯｡後☆繧菊xe
        filePath.c_str(),// 蠑墓焚・医％縺薙〒縺ｯ髢九″縺溘＞繝輔ぃ繧､繝ｫ繝代せ・・
        NULL,           // 繧ｫ繝ｬ繝ｳ繝医ョ繧｣繝ｬ繧ｯ繝医Μ
        SW_SHOWNORMAL   // 繧ｦ繧｣繝ｳ繝峨え陦ｨ遉ｺ譁ｹ豕・
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
		ofs << json.dump(4); // 繧､繝ｳ繝・Φ繝亥ｹ・縺ｧ謨ｴ蠖｢縺励※菫晏ｭ・
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
