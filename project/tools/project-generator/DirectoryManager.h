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

		/// @brief Set the root directory path.
		bool SetLootDirectory(const std::string& path);
		/// @brief Apply results from the most recent directory scan when it completes.
		void Update();
		/// @brief Get the root directory path.
		const std::string& GetLootDirectory() const;
		/// @brief Get the directories below the root.
		const std::vector<DirectoryEntry>& GetDirectories() const;
		/// @brief Get the directory scan state.
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
