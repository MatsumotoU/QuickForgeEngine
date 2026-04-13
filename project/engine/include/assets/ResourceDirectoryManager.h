#pragma once
#include <string>
#include <unordered_map>

namespace QFE {
	/// @class ResourceDirectoryManager
	/// @brief リソースタイプと対応するディレクトリを管理するクラス
	class ResourceDirectoryManager final {
	public:
		ResourceDirectoryManager();
		~ResourceDirectoryManager() = default;

		/// @brief 新しいプロジェクトのディレクトリを生成する関数
		void CreateProjectDirectory(const std::string& projectName) const;
		/// @brief プロジェクトのディレクトリが存在するか確認してなければディレクトリを生成して、プロジェクト名を設定する関数
		void SetProjectDirectory(const std::string& projectName);
		/// brief プロジェクト名を設定する関数
		void SetProjectName(const std::string& projectName) { ProjectName_ = projectName; }

		/// @brief プロジェクト名を取得する関数
		std::string GetProjectName() const { return ProjectName_; }
		/// @brief プロジェクトのディレクトリを取得する関数
		std::string GetProjectDirectory() const;
		/// @brief 指定されたリソースタイプに対応するディレクトリを取得する関数
		std::string GetResourceDirectory(const std::string& resourceType) const;
		/// @brief ルートのディレクトリを取得する関数
		std::string GetRootDirectory() const { return rootDirectory_; }

		/// @brief エディタ用のリソースディレクトリを取得する関数
		std::string GetEditorResourceDirectory() const;

		/// @brief ファイルのディレクトリの整合性を確認する関数
		bool CheckDirectoryIntegrity() const;
		/// @brief ディレクトリの整合性を修復する関数
		void RepairDirectoryIntegrity() const;

	private:
		std::string ProjectName_;
		std::string rootDirectory_;
		std::unordered_map<std::string, std::string> resourceDirectories_;
	};

}
