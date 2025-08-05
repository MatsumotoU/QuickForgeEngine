#include "FileLoader.h"
#include <fstream>
#include <sstream>
#include <assert.h>

#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

std::string FileLoader::ReadFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary);
    if (!file) {
#ifdef _DEBUG
		DebugLog("Failed to open file: " + filePath);
		assert(false && "Failed to open file");
#endif // _DEBUG
        return std::string();
    }
    std::ostringstream contents;
    contents << file.rdbuf();
#ifdef _DEBUG
    DebugLog("Read file: "+filePath);
	DebugLog("File contents: " + contents.str());
	if (contents.str().empty()) {
		DebugLog("File is empty: " + filePath);
	} else {
		DebugLog("File size: " + std::to_string(contents.str().size()) + " bytes");
	}
#endif // _DEBUG
    return contents.str();
}

std::vector<std::string> FileLoader::Split(const std::string& str, char delimiter) {
    if (str.empty()) {
#ifdef _DEBUG
        DebugLog("The string to split is empty");
#endif // _DEBUG
        return std::vector<std::string>();
    }

#ifdef _DEBUG
    DebugLog("Splitting string: " + str + " with delimiter: " + delimiter);
#endif // _DEBUG
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, delimiter)) {
        // 改行文字を除去
        token.erase(std::remove(token.begin(), token.end(), '\n'), token.end());
        token.erase(std::remove(token.begin(), token.end(), '\r'), token.end());
        if (!token.empty()) {
            result.push_back(token);
#ifdef _DEBUG
            DebugLog("Split: " + token);
#endif // _DEBUG
        }
    }
    return result;
}

std::string FileLoader::OpenFileDialog() {
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameA(&ofn)) {
        return filename;
    }
    return "";
}

std::vector<std::string> FileLoader::GetFilesWithExtension(const std::string& directoryPath, const std::string& extension) {
    std::vector<std::string> files;
    std::string searchPath = directoryPath + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string filename = findData.cFileName;
                // 拡張子が一致するかチェック（大文字小文字区別なし）
                if (filename.length() >= extension.length()) {
                    std::string fileExt = filename.substr(filename.length() - extension.length());
                    std::string extLower = extension;
                    // ここを修正
                    std::transform(fileExt.begin(), fileExt.end(), fileExt.begin(),
                        [](char c) { return static_cast<char>(tolower(static_cast<unsigned char>(c))); });
                    std::transform(extLower.begin(), extLower.end(), extLower.begin(),
                        [](char c) { return static_cast<char>(tolower(static_cast<unsigned char>(c))); });
                    if (fileExt == extLower) {
                        files.emplace_back(filename);
                    }
                }
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
    return files;
}

std::string FileLoader::ExtractFileName(const std::string& fullPath) {
    return std::filesystem::path(fullPath).stem().string();
}
