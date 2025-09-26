#pragma once
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <system_error>

namespace QFE::FILE {
	extern std::vector<std::string> GetFilesInDirectory(const std::string& directoryPath, const std::string& extension = "");
}