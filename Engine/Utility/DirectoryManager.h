#pragma once
#include <string>
#include <array>

class DirectoryManager final{
public:
	enum  DirectoryType {
		ModelDirectory,
		ImageDirectory,
		AudioDirectory,
		DirectoryTypeCount
	};
	/// <summary>
	/// 各種リソースのディレクトリパスを取得する
	/// </summary>
	/// <param name="type"></param>
	/// <returns></returns>
	const std::string& GetDirectoryPath(DirectoryType type) const{
		return directories_[type];

	}
private:
	const std::string basePath_ = "Resource/";
	std::array<std::string, DirectoryTypeCount> directories_ = {
		basePath_ + "Models/",
		basePath_ + "Images/",
		basePath_ + "Sounds/"
	};
};