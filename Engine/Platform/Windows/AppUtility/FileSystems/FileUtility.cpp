#include "FileUtility.h"
#include <cassert>

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
