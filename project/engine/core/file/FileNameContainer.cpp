#include "FileNameContainer.h"
#include "FileUtility.h"

using namespace QFE::FILE;

void FileNameContainer::Initialize() {
	fileNamesByExtension_.clear();
}

void FileNameContainer::AddFileName(const std::string& fileName) {
	// ファイル名から拡張子を抽出
	size_t lastDotPos = fileName.find_last_of('.');
	if (lastDotPos == std::string::npos) {
		return;
	}
	std::string extension = fileName.substr(lastDotPos);

	// ディレクトリならファイル名を抽出
	std::string fileNameOnly = GetFileName(fileName);
	fileNamesByExtension_[extension].push_back(fileNameOnly);
}

void FileNameContainer::AddFileNamesByDirectory(const std::string& directoryPath) {
	std::vector<std::string> files = GetFilesInDirectory(directoryPath);
	for (const std::string& file : files) {
		AddFileName(file);
	}
}

void FileNameContainer::AddFileNamesByExtension(const std::string& directoryPath, const std::string& extension) {
	std::vector<std::string> files = GetFilesInDirectory(directoryPath, extension);
	for (const std::string& file : files) {
		// ファイル名から拡張子を抽出
		size_t lastDotPos = file.find_last_of('.');
		if (lastDotPos == std::string::npos) {
			return;
		}
		std::string fileExtension = file.substr(lastDotPos);

		// 拡張子が一致しない場合はスキップ
		if(fileExtension != extension) {
			continue;
		}
		AddFileName(file);
	}
}

void FileNameContainer::AddFileNamesByDirectoryRecursively(const std::string& directoryPath) {

}

void FileNameContainer::AddFileNamesByExtensionRecursively(const std::string& directoryPath, const std::string& extension) {
}
