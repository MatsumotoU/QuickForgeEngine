#include "FileLoader.h"
#include <fstream>
#include <sstream>
#include <assert.h>

#ifdef _DEBUG
#include "Base/MyDebugLog.h"
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