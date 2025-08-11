#include "CheckLoadFileExtension.h"
#ifdef _DEBUG
#include "Utility/MyDebugLog.h"
#endif // _DEBUG

void CheckLoadFileExtension::Check(std::string& fileName, const std::string& extension) {
	if (fileName.size() < extension.size() || fileName.substr(fileName.size() - extension.size()) != extension) {
		fileName += extension; // 拡張子がない場合は追加
	}
#ifdef _DEBUG
	DebugLog(std::format("CheckLoadFileExtension: {}", fileName));
#endif // _DEBUG
}
