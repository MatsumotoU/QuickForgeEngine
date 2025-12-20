#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <system_error>
#include <nlohmann/json.hpp>

namespace QFE::FILE {
	extern std::vector<std::string> GetFilesInDirectory(const std::string& directoryPath, const std::string& extension = "");
	extern bool OpenFileOnExe(const std::string& exePath, const std::string& filePath);
	extern bool LoadFileToJson(const std::string& filePath, nlohmann::json& json);
	bool HasExtension(const std::string& fileName, const std::string& extension);

	extern bool LoadCSVToVector(const std::string& filePath, std::vector<std::vector<uint32_t>>& map);
	extern bool SaveJSONToFile(const std::string& filePath, const nlohmann::json& json);
}
