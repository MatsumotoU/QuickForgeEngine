#pragma once
#include <cstdint>
#include <filesystem>
#include <future>
#include <string>
#include <vector>

namespace QFE::APPLICATION
{
	struct DirectoryEntry
	{
		std::filesystem::path absolutePath;
		std::filesystem::path relativePath;
		std::string name;
		std::uint32_t depth = 0;
	};

	enum class DirectoryScanState
	{
		Idle,
		Scanning,
		Ready,
		Failed,
	};

	class DirectoryManager
	{
	public:
		void Initialize();

		/// @brief Lootディレクトリのパスをセットする
		bool SetLootDirectory(const std::string& path);
		/// @brief 前回開始したディレクトリ走査の完了を確認し、結果を反映する
		void Update();
		/// @brief Lootディレクトリのパスを取得する
		const std::string& GetLootDirectory() const;
		/// @brief ルート配下のディレクトリ一覧を取得する
		const std::vector<DirectoryEntry>& GetDirectories() const;
		/// @brief ディレクトリ走査の状態を取得する
		DirectoryScanState GetScanState() const;




	private:
		static std::vector<DirectoryEntry> ScanDirectoryTree(
			const std::filesystem::path& rootPath);
		bool IsScanRunning() const;

		std::string lootDirectory_;
		std::vector<DirectoryEntry> directories_;
		DirectoryScanState scanState_ = DirectoryScanState::Idle;
		std::future<std::vector<DirectoryEntry>> directoryScan_;
	};
}
