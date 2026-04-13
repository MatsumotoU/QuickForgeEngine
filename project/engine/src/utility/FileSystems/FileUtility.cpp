#include "engine/include/utility/FileSystems/FileUtility.h"
#include <cassert>
#include <windows.h>

namespace QFE::FILE {

	std::vector<std::string> GetFilesInDirectory(const std::string& directoryPath, const std::string& extension) {
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

	std::vector<std::string> GetDirectoriesInDirectory(const std::string& directoryPath) {
		std::vector<std::string> directories;
		namespace fs = std::filesystem;
		for (const auto& entry : fs::directory_iterator(directoryPath)) {
			if (entry.is_directory()) {
				directories.push_back(entry.path().filename().string());
			}
		}
		return directories;
	}

	bool OpenFileOnExe(const std::string& exePath, const std::string& filePath) {
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

	bool LoadFileToJson(const std::string& filePath, nlohmann::json& json) {
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

	bool HasExtension(const std::string& fileName, const std::string& extension) {
		if (fileName.empty() || extension.empty()) {
			return false;
		}
		return fileName.size() >= extension.size() &&
			fileName.compare(fileName.size() - extension.size(), extension.size(), extension) == 0;
	}

	bool LoadCSVToVector(const std::string& filePath, std::vector<std::vector<uint32_t>>& map) {
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

	bool SaveJSONToFile(const std::string& filePath, const nlohmann::json& json) {
		std::ofstream ofs(filePath);
		if (ofs.is_open()) {
			ofs << json.dump(4); // インデント幅4で整形して保存
			ofs.close();
			return true;
		}
		return false;
	}

	std::string WideToUTF8(const std::wstring& wstr)
	{
		if (wstr.empty()) return {};
		int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		std::string result(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], size, nullptr, nullptr);
		return result;
	}

	std::wstring GetAbsolutePath(const std::wstring& relativePath) {
		wchar_t fullPath[MAX_PATH];
		DWORD ret = GetFullPathNameW(relativePath.c_str(), MAX_PATH, fullPath, nullptr);
		if (ret == 0 || ret > MAX_PATH) {
			return relativePath; // 失敗時はそのまま返す
		}
		return std::wstring(fullPath);
	}

	bool SaveJsonAsMsgPack(const nlohmann::json& jsonFile, const std::string& msgPackSavePath) {
		nlohmann::json::binary_t msgPackData = nlohmann::json::to_msgpack(jsonFile);
		std::ofstream ofs(msgPackSavePath, std::ios::binary);
		if (ofs.is_open()) {
			ofs.write(reinterpret_cast<const char*>(msgPackData.data()), msgPackData.size());
			ofs.close();
			return true;
		}
		return false;
	}

	bool LoadMsgPackToJson(const std::string& msgPackFilePath, nlohmann::json& json) {
		std::ifstream ifs(msgPackFilePath, std::ios::binary);
		if (ifs.is_open()) {
			std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			ifs.close();
			try {
				json = nlohmann::json::from_msgpack(buffer);
				return true;
			}
			catch (const nlohmann::json::parse_error& e) {
				e;
				assert(false && e.what());
			}
		}
		return false;
	}

	bool HasDirectory(const std::string& directoryPath) {
		return std::filesystem::exists(directoryPath) && std::filesystem::is_directory(directoryPath);
	}

}
