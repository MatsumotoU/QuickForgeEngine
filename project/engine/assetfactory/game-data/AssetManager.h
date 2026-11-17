#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace QFE::ASSET {
	/// プロジェクト内の生ファイルの取り込みと一覧を扱う。
	class AssetManager final {
	public:
		bool Open(const std::filesystem::path& projectRoot, std::string& error);
		bool ImportRawFile(const std::filesystem::path& source, std::string& error);
		std::vector<std::string> ListProjectFiles(std::string& error) const;
	private:
		std::filesystem::path root_;
	};
}
