#pragma once
#include <string>
#include <list>
#include <unordered_map>

namespace QFE::FILE {
	/// @brief ファイル名を格納するクラス
	class FileNameContainer final {
	public:
		/// @brief コンテナを初期化します。
		void Initialize();
		/// @brief ディレクトリを含むパスからファイル名を追加します。拡張子ごとにファイル名を管理します。
		void AddFileName(const std::string& fileName);
		/// @brief 指定のディレクトリにあるファイル名を取得します。
		void AddFileNamesByDirectory(const std::string& directoryPath);
		/// @brief 指定のディレクトリにある特定の拡張子を持つファイル名を取得します。
		void AddFileNamesByExtension(const std::string& directoryPath, const std::string& extension);
		/// @brief 指定のディレクトリからすべての子ディレクトリを含むファイル名を取得します。
		void AddFileNamesByDirectoryRecursively(const std::string& directoryPath);
		/// @brief 指定のディレクトリからすべての子ディレクトリを含む特定の拡張子を持つファイル名を取得します。
		void AddFileNamesByExtensionRecursively(const std::string& directoryPath, const std::string& extension);

	private:
		std::unordered_map<std::string, std::list<std::string>> fileNamesByExtension_;
	};
}